#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "pd9p.h"

pd9p_session*
pd9p_connect(int fd) {
	char versiondata[12], *p, rcmd, rdata[8192], *rversion, *attachdata, *uname;
	uint16_t rtag, rversionlen, unamelen;
	uint32_t rdatalen, rmsize;
	pd9p_session s, *ret;
	
	s.fd=fd;
	s.msize=1<<13;
	s.freefids=0;
	s.fidcounter=0;
	
	p=pd9p_enc4(versiondata, s.msize);
	p=pd9p_enc2(p, 6);
	memcpy(p, "9P2000", 6);
	
	if(pd9p_send(&s, Tversion, notag, 12, versiondata) == -1)
		return 0;
	
	
	if(pd9p_recv(&s, &rcmd, &rtag, &rdatalen, rdata) == -1)
		return 0;
	if(rcmd!=Rversion)
		return 0;
	if(rdatalen<6)
		return 0;
	
	p=pd9p_dec4(rdata, &rmsize);
	if(rmsize<s.msize)
		s.msize=rmsize;
	
	p=pd9p_dec2(p, &rversionlen);
	if(rdatalen < (uint16_t)(6+rversionlen))
		return 0;
	
	rversion=malloc(rversionlen+1);
	memcpy(rversion, p, rversionlen);
	rversion[rversionlen]=0;
	
	if(strcmp(rversion, "9P2000")) {
		free(rversion);
		return 0;
	}
	free(rversion);
	
	
	if((uname=getenv("LOGNAME")) == 0)
		uname="";
	unamelen=strlen(uname);
	attachdata=malloc(4+4+2+unamelen+2);
	s.rootfid=pd9p_newfid(&s);
	p=pd9p_enc4(attachdata, s.rootfid);
	p=pd9p_enc4(p, nofid);
	p=pd9p_enc2(p, unamelen);
	memcpy(p, uname, unamelen);
	p+=unamelen;
	pd9p_enc2(p, 0);
	
	if(pd9p_send(&s, Tattach, 0, 4+4+2+unamelen+2, attachdata) == -1) {
		free(attachdata);
		return 0;
	}
	free(attachdata);
	
	
	if(pd9p_recv(&s, &rcmd, &rtag, &rdatalen, rdata) == -1)
		return 0;
		
	if(rcmd!=Rattach)
		return 0;
	if(rdatalen!=13)
		return 0;
	if(rtag!=0)
		return 0;
	
	ret=malloc(sizeof(pd9p_session));
	memcpy(ret, &s, sizeof(pd9p_session));
	return ret;
}

void
pd9p_closesession(pd9p_session *s) {
	struct pd9p_fidlinklist *fllp;
	struct pd9p_fidlinklist *nextfllp;
	/* Clean all linked lists */
	for(fllp=(*s).freefids; fllp; fllp=nextfllp) {
		nextfllp=(*fllp).next;
		free(fllp);
	}
	
	free(s);
}

uint32_t
pd9p_getfid(pd9p_session *s, char *path) {
	uint16_t walkslen, i, componentlen, rtag;
	char **walks, *p, *component, *buf, *walkdata, rcmd, *rdata;
	uint32_t fid, walkdatalen, rdatalen;
	
	walks=malloc(0);
	walkslen=0;
	for(component=p=path, componentlen=0; ; p++) {
		if(*p=='/' || *p==0) {
			if(componentlen) {
				if((buf=malloc(componentlen+1)) == 0)
					exit(1);
				memcpy(buf, component, componentlen);
				buf[componentlen]=0;
				
				if((walks=realloc(walks, ++walkslen*sizeof(char*))) == 0)
					exit(1);
				
				walks[walkslen-1]=buf;
			}
			componentlen=0;
			component=p+1;
			
			if(*p==0)
				break;
		} else
			componentlen++;
	}
	
	for(walkdatalen=10, i=0; i<walkslen; i++)
		walkdatalen+=2+strlen(walks[i]);
	
	if((walkdata=malloc(walkdatalen)) == 0)
		exit(1);
	
	fid=pd9p_newfid(s);
	p=pd9p_enc4(walkdata, (*s).rootfid);
	p=pd9p_enc4(p, fid);
	p=pd9p_enc2(p, walkslen);
	
	for(i=0; i<walkslen; i++) {
		componentlen=strlen(walks[i]);
		p=pd9p_enc2(p, componentlen);
		memcpy(p, walks[i], componentlen);
		p+=componentlen;
		free(walks[i]);
	}
	free(walks);
	
	if(pd9p_send(s, Twalk, 0, walkdatalen, walkdata) == -1) {
		free(walkdata);
		return -1;
	}
	free(walkdata);
	
	rdata=malloc((*s).msize);
	if(pd9p_recv(s, &rcmd, &rtag, &rdatalen, rdata) == -1) {
		free(rdata);
		return -1;
	}
	free(rdata);
	
	if(rcmd!=Rwalk)
		return -1;
	if(rtag!=0)
		return -1;
	
	return fid;
}

uint32_t
pd9p_open(pd9p_session *s, char *path, char mode) {
	uint32_t fid, rdatalen;
	uint16_t rtag;
	char *p, opendata[5], rcmd, *rdata;
	
	if((fid=pd9p_getfid(s, path)) == errfid)
		return -1;
	p=pd9p_enc4(opendata, fid);
	pd9p_enc1(p, mode);
	
	if(pd9p_send(s, Topen, 0, 5, opendata) == -1)
		return -1;
	
	rdata=malloc((*s).msize);
	if(pd9p_recv(s, &rcmd, &rtag, &rdatalen, rdata) == -1) {
		free(rdata);
		return -1;
	}
	
	if(rcmd!=Ropen)
		return -1;
	if(rtag!=0)
		return -1;
	if(rdatalen!=17)
		return -1;
	/* FIXME: iounit is not handled as of now */
	free(rdata);
	
	return fid;
}

int
pd9p_close(pd9p_session *s, uint32_t fid) {
	uint32_t rdatalen;
	uint16_t rtag;
	char closedata[4], rcmd, *rdata;
	
	pd9p_delfid(s, fid);
	pd9p_enc4(closedata, fid);
	
	if(pd9p_send(s, Tclunk, 0, 4, closedata) == -1)
		return -1;
	
	rdata=malloc((*s).msize);
	if(pd9p_recv(s, &rcmd, &rtag, &rdatalen, rdata) == -1) {
		free(rdata);
		return -1;
	}
	
	if(rcmd!=Rclunk) {
		free(rdata);
		return -1;
	}
	if(rtag!=0) {
		free(rdata);
		return -1;
	}
	if(rdatalen!=0) {
		free(rdata);
		return -1;
	}
	free(rdata);
	
	return 0;
}

int32_t
pd9p_read(pd9p_session *s, uint32_t fid, char *buf, uint32_t count) {
	uint32_t rdatalen, segmentlen;
	uint16_t rtag;
	char readdata[16], *p, rcmd, *rdata;
	
	/* FIXME: offset is ignored */
	p=pd9p_enc4(readdata, fid);
	p=pd9p_enc4(p, 0);
	p=pd9p_enc4(p, 0);
	p=pd9p_enc4(p, count);
	
	if(pd9p_send(s, Tread, 0, 16, readdata) == -1)
		return -1;
	
	if((rdata=malloc((*s).msize)) == 0)
		return -1;
	if(pd9p_recv(s, &rcmd, &rtag, &rdatalen, rdata) == -1) {
		free(rdata);
		return -1;
	}
	
	if(rcmd!=Rread) {
		free(rdata);
		return -1;
	}
	if(rtag!=0) {
		free(rdata);
		return -1;
	}
	if(rdatalen<4) {
		free(rdata);
		return -1;
	}
	
	p=pd9p_dec4(rdata, &segmentlen);
	if(rdatalen<segmentlen+4)
		return -1;
	if(count<segmentlen)
		return -1;
	memcpy(buf, p, segmentlen);
	
	free(rdata);
	
	return segmentlen;
}

int32_t
pd9p_write(pd9p_session *s, uint32_t fid, char *buf, uint32_t count) {
	uint32_t rdatalen, datalen, segmentlen;
	uint16_t rtag;
	char *writedata, *p, rcmd, *rdata;
	
	if(count>(*s).msize-16)
		datalen=(*s).msize-16;
	else
		datalen=count;
	
	if((writedata=malloc(16+datalen)) == 0)
		return -1;
	
	/* FIXME: offset is ignored */
	p=pd9p_enc4(writedata, fid);
	p=pd9p_enc4(p, 0);
	p=pd9p_enc4(p, 0);
	p=pd9p_enc4(p, datalen);
	memcpy(p, buf, datalen);
	
	if(pd9p_send(s, Twrite, 0, datalen, writedata) == -1) {
		free(writedata);
		return -1;
	}
	
	if((rdata=malloc((*s).msize)) == 0)
		return -1;
	
	if(pd9p_recv(s, &rcmd, &rtag, &rdatalen, rdata) == -1) {
		free(rdata);
		return -1;
	}
	
	if(rcmd!=Rwrite) {
		free(rdata);
		return -1;
	}
	if(rtag!=0) {
		free(rdata);
		return -1;
	}
	if(rdatalen!=4) {
		free(rdata);
		return -1;
	}
	
	pd9p_dec4(rdata, &segmentlen);
	free(rdata);
	
	return segmentlen;
}
