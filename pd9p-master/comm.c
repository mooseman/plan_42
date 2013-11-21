#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pd9p.h"

int
pd9p_send(pd9p_session *s, char cmd, uint16_t tag, uint32_t datalen, char *data) {
	char *buf, *p;
	uint32_t sent, size;
	
	size=4+1+2+datalen;
	if(size>(*s).msize)
		return -1;
	if((buf=malloc(size)) == 0)
		exit(1);
	p=pd9p_enc4(buf, size);
	p=pd9p_enc1(p, cmd);
	p=pd9p_enc2(p, tag);
	memcpy(p, data, datalen);
	
	for(sent=0; sent<size;)
		sent+=write((*s).fd, buf+sent, size-sent);
	
	free(buf);
	return sent;
}

int
pd9p_recv(pd9p_session *s, char *cmd, uint16_t *tag, uint32_t *datalen, char *data) {
	char tmpdata[4];
	uint32_t recvd, size;
	
	for(recvd=0; recvd<4;)
		recvd+=read((*s).fd, tmpdata, 4-recvd);
	pd9p_dec4(tmpdata, &size);
	*datalen=size-4-1-2;
	
	read((*s).fd, cmd, 1);
	
	for(recvd=0; recvd<2;)
		recvd+=read((*s).fd, tmpdata, 2-recvd);
	pd9p_dec2(tmpdata, tag);
	
	for(recvd=0; recvd < *datalen;)
		recvd+=read((*s).fd, data, *datalen-recvd);
	return recvd;
}
