/*
   mkp42img.c – minimal GPT‑disk + FAT32 ESP image builder
   -------------------------------------------------------
   This code is released to the public domain.  
   "Share and enjoy....."  :)  
*/

#define _GNU_SOURCE
#include <ctype.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

/* ----------------------------------------------------------------- helpers */
static void die(const char *msg) { perror(msg); exit(EXIT_FAILURE); }
static uint32_t crc32(const void *data, size_t len);   /* declaration below */

/* Round x up to next multiple of a (a power of 2) */
#define ALIGN(x,a)  (((x) + (a) - 1) & ~((a) - 1))

/* Little‑endian store helpers */
static void le16(void *p, uint16_t v) { *(uint16_t*)p = htole16(v); }
static void le32(void *p, uint32_t v) { *(uint32_t*)p = htole32(v); }
static void le64(void *p, uint64_t v) { *(uint64_t*)p = htole64(v); }

/* ----------------------------------------------------------------- structs */
#pragma pack(push,1)
struct chs { uint8_t h,s,l; };
struct mbr_part {
    uint8_t  status;
    struct chs first_chs;
    uint8_t  type;
    struct chs last_chs;
    uint32_t first_lba;
    uint32_t len;
};
struct mbr {
    uint8_t  boot[440];
    uint32_t sig;
    uint16_t pad;
    struct mbr_part part[4];
    uint16_t magic;
};

struct gpt_header {
    char     sig[8];
    uint32_t rev;
    uint32_t hdr_sz;
    uint32_t hdr_crc;
    uint32_t reserved;
    uint64_t hdr_lba;
    uint64_t alt_lba;
    uint64_t first_usable;
    uint64_t last_usable;
    uint8_t  disk_guid[16];
    uint64_t table_lba;
    uint32_t num_entries;
    uint32_t entry_size;
    uint32_t table_crc;
    uint8_t  pad[420];
};

struct gpt_entry {
    uint8_t  type_guid[16];
    uint8_t  part_guid[16];
    uint64_t first_lba;
    uint64_t last_lba;
    uint64_t attrs;
    uint16_t name[36];
};
#pragma pack(pop)

/* ESP type GUID = C12A7328-F81F-11D2-BA4B-00A0C93EC93B (little‑endian) */
static const uint8_t esp_guid[16] = {
 0x28,0x73,0x2A,0xC1, 0x1F,0xF8, 0xD2,0x11,
 0xBA,0x4B, 0x00,0xA0,0xC9,0x3E,0xC9,0x3B };

/* ---------------------------------------------------------------- FAT32 */
enum { SECTOR_SIZE = 512, CLUSTER_SIZE = 4096 }; /* 8 sectors per cluster */
enum { RESERVED = 32, NUM_FATS = 2 };

static void write_fat32(int fd, uint64_t first_lba, uint64_t tot_secs)
{
    /* compute FAT size */
    uint32_t data_secs = tot_secs - RESERVED;
    uint32_t clusters  = data_secs / (CLUSTER_SIZE/SECTOR_SIZE);
    uint32_t fat_secs  = ALIGN((clusters+2)*4, SECTOR_SIZE) / SECTOR_SIZE;
    data_secs          = tot_secs - RESERVED - NUM_FATS*fat_secs;
    clusters           = data_secs / (CLUSTER_SIZE/SECTOR_SIZE);

    off_t off = first_lba * SECTOR_SIZE;
    uint8_t *buf = calloc(1, SECTOR_SIZE); if(!buf)die("calloc");

    /* --- BPB / VBR ------------------------------------------------------ */
    buf[0] = 0xEB; buf[1]=0x58; buf[2]=0x90; memcpy(buf+3,"MSDOS5.0",8);
    le16(buf+11, SECTOR_SIZE);
    buf[13] = CLUSTER_SIZE/SECTOR_SIZE;
    le16(buf+14, RESERVED);
    buf[16] = NUM_FATS;
    le32(buf+32, fat_secs);                   /* FAT size 32 */
    le32(buf+36, RESERVED + NUM_FATS*fat_secs); /* root cluster (#2) */
    le16(buf+40, 1);                          /* FS version */
    le16(buf+42, 1);                          /* FSInfo sector */
    le16(buf+44, 6);                          /* backup VBR */
    memcpy(buf+82,"SEA-DOS ",8);
    buf[510]=0x55; buf[511]=0xAA;
    if(pwrite(fd,buf,SECTOR_SIZE,off)!=SECTOR_SIZE)die("vbr");

    /* FSInfo */
    memset(buf,0,SECTOR_SIZE);
    le32(buf+0,0x41615252); le32(buf+484,0x61417272);
    le32(buf+488,clusters-1); le32(buf+492,3);
    buf[510]=0x55; buf[511]=0xAA;
    if(pwrite(fd,buf,SECTOR_SIZE,off+SECTOR_SIZE)!=SECTOR_SIZE)die("fsinfo");

    /* FAT[0] */
    memset(buf,0,SECTOR_SIZE);
    le32(buf+0,0x0FFFFFF8);   /* cluster 0 (media)   */
    le32(buf+4,0xFFFFFFFF);   /* cluster 1 (reserved)*/
    le32(buf+8,0x0FFFFFFF);   /* cluster 2 (root dir)*/
    if(pwrite(fd,buf,SECTOR_SIZE,
              off+RESERVED*SECTOR_SIZE)!=SECTOR_SIZE)die("fat0");

    free(buf);
}

/* ---------------------------------------------------------------- main */
int main(int argc,char **argv)
{
    const char *out=NULL;
    int size_mib=64;
    enum{MAX_ITEMS=128};
    struct{int is_efi;char *src;char *dst;}items[MAX_ITEMS];
    int nitems=0;

    /* CLI --------------------------------------------------------------- */
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"-o")&&i+1<argc){out=argv[++i];continue;}
        if(!strcmp(argv[i],"-s")&&i+1<argc){size_mib=atoi(argv[++i]);continue;}
        if(!strcmp(argv[i],"-ae")&&i+2<argc){
            items[nitems++] = (typeof(items[0])){1,argv[i+2],argv[i+1]}; i+=2;continue;}
        if(!strcmp(argv[i],"-ad")&&i+2<argc){
            items[nitems++] = (typeof(items[0])){0,argv[i+1],argv[i+2]}; i+=2;continue;}
        fprintf(stderr,"Usage: mkp42img -o img [-s MiB] "
                       " -ae UEFIpath host  |  -ad host dest\n"); return 1;
    }
    if(!out)die("output missing");

    /* create image file ------------------------------------------------- */
    uint64_t tot_secs = (uint64_t)size_mib*1024*1024/SECTOR_SIZE;
    int fd=open(out,O_RDWR|O_CREAT|O_TRUNC,0666); if(fd<0)die("open");
    if(ftruncate(fd,(off_t)size_mib*1024*1024))die("truncate");

    /* Protective MBR ---------------------------------------------------- */
    struct mbr m={.magic=0xAA55};
    m.part[0].type=0xEE; m.part[0].first_lba=1; m.part[0].len=0xFFFFFFFF;
    if(pwrite(fd,&m,sizeof m,0)!=sizeof m)die("mbr");

    /* GPT header + full table ------------------------------------------ */
    struct gpt_header h={0};
    memcpy(h.sig,"EFI PART",8); h.rev=0x00010000; h.hdr_sz=92;
    h.hdr_lba=1; h.first_usable=2048; h.last_usable=tot_secs-1;
    h.table_lba=2; h.num_entries=128; h.entry_size=128;

    uint8_t tbl[128*128]={0};
    struct gpt_entry *e=(struct gpt_entry*)tbl;
    e->first_lba=2048; e->last_lba=tot_secs-1;
    memcpy(e->type_guid,esp_guid,16);
    e->name[0]='E'; e->name[1]='S'; e->name[2]='P';

    h.table_crc=crc32(tbl,sizeof tbl);
    if(pwrite(fd,tbl,sizeof tbl,h.table_lba*SECTOR_SIZE)!=sizeof tbl)die("gpt tbl");

    h.hdr_crc=0;
    h.hdr_crc=crc32(&h,h.hdr_sz);
    if(pwrite(fd,&h,sizeof h,SECTOR_SIZE)!=sizeof h)die("gpt hdr");

    /* FAT32 ESP --------------------------------------------------------- */
    uint64_t part_secs = tot_secs - 2048;
    write_fat32(fd,2048,part_secs);

    /* Copy files -------------------------------------------------------- */
    off_t data_off=(2048+RESERVED+NUM_FATS*32)*SECTOR_SIZE;
    uint8_t dirent[32]={0};
    for(int i=0;i<nitems;i++){
        int f=open(items[i].src,O_RDONLY); if(f<0)die(items[i].src);
        struct stat st; if(fstat(f,&st))die("stat");
        void *buf=mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,f,0);
        if(buf==MAP_FAILED)die("mmap");

        /* allocate one cluster/file (limit 4 KiB) */
        uint32_t clu=3+i;
        off_t clu_off=data_off+(clu-2)*CLUSTER_SIZE;
        if(pwrite(fd,buf,st.st_size,clu_off)!=st.st_size)die("write file");
        munmap(buf,st.st_size); close(f);

        memset(dirent,' ',11);
        const char *n=strrchr(items[i].dst,'/'); n=n?n+1:items[i].dst;
        int j=0; while(*n&&*n!='.'&&j<8)dirent[j++]=toupper(*n++);
        if(*n=='.'){j=8;n++;int k=0;while(*n&&k<3)dirent[j++]=toupper(*n++);}
        dirent[11]=items[i].is_efi?0x07:0x20;
        le16(dirent+20,clu>>16); le16(dirent+26,clu&0xFFFF);
        le32(dirent+28,st.st_size);
        if(pwrite(fd,dirent,32,data_off+(i*32))!=32)die("dirent");
    }

    close(fd);
    puts("mkp42img: finished.");
    return 0;
}

/* -------------------- tiny CRC‑32 --------------------------------------- */
static uint32_t crc32(const void *data,size_t len)
{
    static uint32_t tbl[256]; static int init=0;
    if(!init){for(uint32_t i=0;i<256;i++){uint32_t c=i;for(int j=0;j<8;j++)
        c=c&1?0xEDB88320u^(c>>1):c>>1; tbl[i]=c;} init=1;}
    uint32_t crc=~0; const uint8_t *p=data;
    while(len--) crc=tbl[(crc^*p++)&0xFF]^crc>>8;
    return ~crc;
}



