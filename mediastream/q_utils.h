/*
  The oRTP library is an RTP (Realtime Transport Protocol - rfc3550) stack.
  Copyright (C) 2001  Simon MORLAT simon.morlat@linphone.org

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
//#include "utils.h"
//#include <basetsd.h>
//#include <windef.h>
//#include <winnt.h>
//#include <winbase.h>

#include "mediastreamer2/msqueue.h"

#ifndef MIN
#define MIN(a,b) (((a)>(b)) ? (b) : (a))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

inline void ms_buff_init(MSBufferizer *obj){
	qinit(&obj->q);
	obj->size=0;
}

inline MSBufferizer * ms_buff_new(){
	MSBufferizer *obj=(MSBufferizer *)malloc(sizeof(MSBufferizer));
	ms_buff_init(obj);
	return obj;
}

inline void ms_buff_put(MSBufferizer *obj, mblk_t *m){
	obj->size+=msgdsize(m);
	putq(&obj->q,m);
}

inline void ms_buff_put_from_queue(MSBufferizer *obj, queue_t *q){
	mblk_t *m;
	while((m=getq(q))!=NULL){
		ms_buff_put(obj,m);
	}
}

inline int ms_buff_read(MSBufferizer *obj, uint8_t *data, int datalen){
	if (obj->size>=datalen){
		int sz=0;
		int cplen;
		mblk_t *m=peekq(&obj->q);
		/*we can return something */
		while(sz<datalen){
			cplen=MIN(m->b_wptr-m->b_rptr,datalen-sz);
			memcpy(data+sz,m->b_rptr,cplen);
			sz+=cplen;
			m->b_rptr+=cplen;
			if (m->b_rptr==m->b_wptr){
				/* check cont */
				if (m->b_cont!=NULL) {
					m=m->b_cont;
				}
				else{
					mblk_t *remove=getq(&obj->q);
					freemsg(remove);
					m=peekq(&obj->q);
				}
			}
		}
		obj->size-=datalen;
		return datalen;
	}
	return 0;
}

inline void ms_buff_uninit(MSBufferizer *obj){
	flushq(&obj->q,0);
}

inline void ms_buff_destroy(MSBufferizer *obj){
	ms_buff_uninit(obj);
	free(obj);
}

/* returns the number of bytes available in the bufferizer*/
static inline int ms_buff_get_avail(MSBufferizer *obj){
	return obj->size;
}
#endif