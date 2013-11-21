#include <stdint.h>
#include <stdlib.h>

#include "pd9p.h"

uint32_t
pd9p_newfid(pd9p_session *s) {
	uint32_t fid;
	struct pd9p_fidlinklist *newp;
	if(!(*s).freefids)
		fid=(*s).fidcounter++;
	else {
		fid=(*(*s).freefids).fid;
		newp=(*(*s).freefids).next;
		free((*s).freefids);
		(*s).freefids=newp;
	}
	return fid;
}

void
pd9p_delfid(pd9p_session *s, uint32_t fid) {
	struct pd9p_fidlinklist *newp;
	if((newp=malloc(sizeof(struct pd9p_fidlinklist))) == 0)
		exit(1);
	(*newp).fid=fid;
	(*newp).next=(*s).freefids;
	(*s).freefids=newp;
}
