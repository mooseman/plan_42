#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "pd9p.h"

int
sockconnect(char *name, uint16_t port) {
	int s;
	struct hostent *he;
	struct sockaddr_in addr;
	
	if((he=gethostbyname(name)) == NULL)
		return -1;
	
	memcpy(&addr.sin_addr, he->h_addr, he->h_length);
	addr.sin_port=htons(port);
	addr.sin_family=AF_INET;
	
	if((s=socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return -1;
	if(connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
		close(s);
		return -1;
	}
	
	return s;
}

int
main(int argc, char **argv) {
	int s, inlen, outlen, f;
	pd9p_session *session;
	uint32_t file;
	char buf[1024];
	
	if(argc<6) {
		fputs("Usage: pd9p addr port cmd remote local\n", stderr);
		return 1;
	}
	
	if((s=sockconnect(argv[1], atoi(argv[2]))) == -1) {
		fputs("pd9p: error: sockconnect\n", stderr);
	}
	
	if((session=pd9p_connect(s)) == 0) {
		close(s);
		fputs("pd9p: error: pd9p_connec\n", stderr);
		return 1;
	}
	
	if(!strcmp(argv[3], "get")) {
		if((file=pd9p_open(session, argv[4], pd9p_rdonly)) == errfid) {
			fputs("pd9p: error: pd9p_open\n", stderr);
			return 1;
		}
		if((f=open(argv[4], O_WRONLY|O_CREAT, 0666)) == -1) {
			fputs("pd9p: error: open\n", stderr);
			return 1;
		}
		
		inlen=0;
		while((inlen=pd9p_read(session, file, buf+inlen, 1024)) > 0) {
			for(outlen=0; outlen<inlen;)
				outlen+=write(f, buf+outlen, inlen-outlen);
		}
		pd9p_close(session, file);
		close(f);
	}
	
	pd9p_closesession(session);
	close(s);
	return 0;
}
