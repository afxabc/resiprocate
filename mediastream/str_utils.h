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

//#include <Windows.h>

#ifndef MIN
#define MIN(a,b) (((a)>(b)) ? (b) : (a))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif


typedef unsigned char uint8_t;

typedef struct msgb
{
	struct msgb *b_prev;
	struct msgb *b_next;
	struct msgb *b_cont;
	struct datab *b_datap;
	unsigned char *b_rptr;
	unsigned char *b_wptr;
	unsigned __int32 reserved1;
	unsigned __int32 reserved2;
} mblk_t;

typedef struct datab
{
	unsigned char *db_base;
	unsigned char *db_lim;
	void (*db_freefn)(void*);
	int db_ref;
} dblk_t;

typedef struct _queue
{
	mblk_t _q_stopper;
	int q_mcount;	/*number of packet in the q */
} queue_t;

#define mblk_set_timestamp_info(m,ts) (m)->reserved1=(ts);
#define mblk_get_timestamp_info(m)    ((m)->reserved1)
#define mblk_set_marker_info(m,bit)   (m)->reserved2=((m)->reserved2|bit)
#define mblk_get_marker_info(m)	      ((m)->reserved2&0x1)
#define mblk_set_rate(m,bits)         (m)->reserved2=((m)->reserved2|(bits)<<1)
#define mblk_get_rate(m)              (((m)->reserved2>>1)&0x3)
#define mblk_set_payload_type(m,bits) (m)->reserved2=((m)->reserved2|(bits<<3))
#define mblk_get_payload_type(m)      (((m)->reserved2>>3)&0x7F)
#define mblk_set_precious_flag(m,bit)    (m)->reserved2=(m)->reserved2|((bit & 0x1)<<10) /*use to prevent mirroring*/
#define mblk_get_precious_flag(m)    (((m)->reserved2)>>10 & 0x1)

#define qempty(q) (&(q)->_q_stopper==(q)->_q_stopper.b_next)
#define qfirst(q) ((q)->_q_stopper.b_next!=&(q)->_q_stopper ? (q)->_q_stopper.b_next : NULL)
#define qbegin(q) ((q)->_q_stopper.b_next)
#define qlast(q) ((q)->_q_stopper.b_prev!=&(q)->_q_stopper ? (q)->_q_stopper.b_prev : NULL)
#define qend(q,mp)	((mp)==&(q)->_q_stopper)
#define qnext(q,mp) ((mp)->b_next)

typedef struct _msgb_allocator
{
	queue_t q;
}msgb_allocator_t;


typedef struct _MSBufferizer
{
	queue_t q;
	int size;
}MSBufferizer;


__inline void mblk_init(mblk_t *mp)
{
	mp->b_cont=mp->b_prev=mp->b_next=NULL;
	mp->b_rptr=mp->b_wptr=NULL;
	mp->reserved1=0;
	mp->reserved2=0;
}

__inline void qinit(queue_t *q){
	mblk_init(&q->_q_stopper);
	q->_q_stopper.b_next=&q->_q_stopper;
	q->_q_stopper.b_prev=&q->_q_stopper;
	q->q_mcount=0;
}

__inline dblk_t *datab_alloc(int size){
	dblk_t *db;
	int total_size=sizeof(dblk_t)+size;
	db=(dblk_t *)malloc(total_size);
	db->db_base=(uint8_t*)db+sizeof(dblk_t);
	db->db_lim=db->db_base+size;
	db->db_ref=1;
	db->db_freefn=NULL;	/* the buffer pointed by db_base must never be freed !*/
	return db;
}

__inline void datab_ref(dblk_t *d){
	d->db_ref++;
}

__inline void datab_unref(dblk_t *d){
	d->db_ref--;
	if (d->db_ref==0){
		if (d->db_freefn!=NULL)
			d->db_freefn(d->db_base);
		free(d);
	}
}


__inline mblk_t *allocb(int size, int pri)
{
	mblk_t *mp;
	dblk_t *datab;
	
	mp=(mblk_t *)malloc(sizeof(mblk_t));
	mblk_init(mp);
	datab=datab_alloc(size);
	
	mp->b_datap=datab;
	mp->b_rptr=mp->b_wptr=datab->db_base;
	mp->b_next=mp->b_prev=mp->b_cont=NULL;
	return mp;
}

__inline mblk_t *esballoc(uint8_t *buf, int size, int pri, void (*freefn)(void*) )
{
	mblk_t *mp;
	dblk_t *datab;
	
	mp=(mblk_t *)malloc(sizeof(mblk_t));
	mblk_init(mp);
	datab=(dblk_t *)malloc(sizeof(dblk_t));
	

	datab->db_base=buf;
	datab->db_lim=buf+size;
	datab->db_ref=1;
	datab->db_freefn=freefn;
	
	mp->b_datap=datab;
	mp->b_rptr=mp->b_wptr=buf;
	mp->b_next=mp->b_prev=mp->b_cont=NULL;
	return mp;
}

	
__inline void freeb(mblk_t *mp)
{
	datab_unref(mp->b_datap);
	free(mp);
}

__inline void freemsg(mblk_t *mp)
{
	mblk_t *tmp1,*tmp2;
	tmp1=mp;
	while(tmp1!=NULL)
	{
		tmp2=tmp1->b_cont;
		freeb(tmp1);
		tmp1=tmp2;
	}
}

__inline mblk_t *dupb(mblk_t *mp)
{
	mblk_t *newm;
	
	datab_ref(mp->b_datap);
	newm=(mblk_t *)malloc(sizeof(mblk_t));
	mblk_init(newm);
	newm->reserved1=mp->reserved1;
	newm->reserved2=mp->reserved2;
	newm->b_datap=mp->b_datap;
	newm->b_rptr=mp->b_rptr;
	newm->b_wptr=mp->b_wptr;
	return newm;
}

/* duplicates a complex mblk_t */
__inline mblk_t	*dupmsg(mblk_t* m)
{
	mblk_t *newm=NULL,*mp,*prev;
	prev=newm=dupb(m);
	m=m->b_cont;
	while (m!=NULL){
		mp=dupb(m);
		prev->b_cont=mp;
		prev=mp;
		m=m->b_cont;
	}
	return newm;
}

__inline void putq(queue_t *q,mblk_t *mp)
{
	q->_q_stopper.b_prev->b_next=mp;
	mp->b_prev=q->_q_stopper.b_prev;
	mp->b_next=&q->_q_stopper;
	q->_q_stopper.b_prev=mp;
	q->q_mcount++;
}

__inline mblk_t *getq(queue_t *q)
{
	mblk_t *tmp;
	tmp=q->_q_stopper.b_next;
	if (tmp==&q->_q_stopper) return NULL;
	q->_q_stopper.b_next=tmp->b_next;
	tmp->b_next->b_prev=&q->_q_stopper;
	tmp->b_prev=NULL;
	tmp->b_next=NULL;
	q->q_mcount--;
	return tmp;
}

__inline mblk_t * peekq(queue_t *q){
	mblk_t *tmp;
	tmp=q->_q_stopper.b_next;
	if (tmp==&q->_q_stopper) return NULL;
	return tmp;
}

/* insert mp in q just before emp */
__inline void insq(queue_t *q,mblk_t *emp, mblk_t *mp)
{
	if (emp==NULL){
		putq(q,mp);
		return;
	}
	q->q_mcount++;
	emp->b_prev->b_next=mp;
	mp->b_prev=emp->b_prev;
	emp->b_prev=mp;
	mp->b_next=emp;	
}

__inline void remq(queue_t *q, mblk_t *mp){
	q->q_mcount--;
	mp->b_prev->b_next=mp->b_next;
	mp->b_next->b_prev=mp->b_prev;
	mp->b_next=NULL;
	mp->b_prev=NULL;
}

/* remove and free all messages in the q */
__inline void flushq(queue_t *q, int how)
{
	mblk_t *mp;
	
	while ((mp=getq(q))!=NULL)
	{
		freemsg(mp);
	}
}

__inline int msgdsize(const mblk_t *mp)
{
	int msgsize=0;
	while(mp!=NULL){
		msgsize+=(int) (mp->b_wptr-mp->b_rptr);
		mp=mp->b_cont;
	}
	return msgsize;
}


__inline void msgpullup(mblk_t *mp,int len)
{
	mblk_t *firstm=mp;
	dblk_t *db;
	int wlen=0;

	if (mp->b_cont==NULL && len==-1) return;	/*nothing to do, message is not fragmented */

	if (len==-1) len=msgdsize(mp);
	db=datab_alloc(len);
	while(wlen<len && mp!=NULL){
		int remain=len-wlen;
		int mlen=mp->b_wptr-mp->b_rptr;
		if (mlen<=remain){
			memcpy(&db->db_base[wlen],mp->b_rptr,mlen);
			wlen+=mlen;
			mp=mp->b_cont;
		}else{
			memcpy(&db->db_base[wlen],mp->b_rptr,remain);
			wlen+=remain;
		}
	}
	/*set firstm to point to the new datab */
	freemsg(firstm->b_cont);
	firstm->b_cont=NULL;
	datab_unref(firstm->b_datap);
	firstm->b_datap=db;
	firstm->b_rptr=db->db_base;
	firstm->b_wptr=firstm->b_rptr+wlen;
}


__inline mblk_t *copyb(mblk_t *mp)
{
	mblk_t *newm;
	int len=(int) (mp->b_wptr-mp->b_rptr);
	newm=allocb(len,0);
	memcpy(newm->b_wptr,mp->b_rptr,len);
	newm->b_wptr+=len;
	return newm;
}

__inline mblk_t *copymsg(mblk_t *mp)
{
	mblk_t *newm=0,*m;
	m=newm=copyb(mp);
	mp=mp->b_cont;
	while(mp!=NULL){
		m->b_cont=copyb(mp);
		m=m->b_cont;
		mp=mp->b_cont;
	}
	return newm;
}

__inline mblk_t * appendb(mblk_t *mp, const char *data, int size, bool pad){
	int padcnt=0;
	int i;
	if (pad){
		padcnt= (int)(4L-( (long)(((long)mp->b_wptr)+size) % 4L)) % 4L;
	}
	if ((mp->b_wptr + size +padcnt) > mp->b_datap->db_lim){
		/* buffer is not large enough: append a new block (with the same size ?)*/
		int plen=(int)((char*)mp->b_datap->db_lim - (char*) mp->b_datap->db_base);
		mp->b_cont=allocb(MAX(plen,size),0);
		mp=mp->b_cont;
	}
	if (size) memcpy(mp->b_wptr,data,size);
	mp->b_wptr+=size;
	for (i=0;i<padcnt;i++){
		mp->b_wptr[0]=0;
		mp->b_wptr++;
	}
	return mp;
}

__inline void msgappend(mblk_t *mp, const char *data, int size, bool pad){
	while(mp->b_cont!=NULL) mp=mp->b_cont;
	appendb(mp,data,size,pad);
}

__inline mblk_t *concatb(mblk_t *mp, mblk_t *newm){
	while (mp->b_cont!=NULL) mp=mp->b_cont;
	mp->b_cont=newm;
	while(newm->b_cont!=NULL) newm=newm->b_cont;
	return newm;
}

__inline void msgb_allocator_init(msgb_allocator_t *a){
	qinit(&a->q);
}

__inline mblk_t *msgb_allocator_alloc(msgb_allocator_t *a, int size){
	queue_t *q=&a->q;
	mblk_t *m,*found=NULL;

	/*lookup for an unused msgb (data block with ref count ==1)*/
	for(m=qbegin(q);!qend(q,m);m=qnext(q,m)){
		if (m->b_datap->db_ref==1 && m->b_datap->db_lim-m->b_datap->db_base>=size){
			found=m;
			break;
		}
	}
	if (found==NULL){
		found=allocb(size,0);
		putq(q,found);
	}
	return dupb(found);
}

__inline void msgb_allocator_uninit(msgb_allocator_t *a){
	flushq(&a->q,-1);
}

__inline void ms_bufferizer_init(MSBufferizer *obj){
	qinit(&obj->q);
	obj->size=0;
}

__inline MSBufferizer * ms_bufferizer_new(){
	MSBufferizer *obj=(MSBufferizer *)malloc(sizeof(MSBufferizer));
	ms_bufferizer_init(obj);
	return obj;
}

__inline void ms_bufferizer_put(MSBufferizer *obj, mblk_t *m){
	obj->size+=msgdsize(m);
	putq(&obj->q,m);
}

__inline void ms_bufferizer_put_from_queue(MSBufferizer *obj, queue_t *q){
	mblk_t *m;
	while((m=getq(q))!=NULL){
		ms_bufferizer_put(obj,m);
	}
}

__inline int ms_bufferizer_read(MSBufferizer *obj, uint8_t *data, int datalen){
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

__inline void ms_bufferizer_uninit(MSBufferizer *obj){
	flushq(&obj->q,0);
}

__inline void ms_bufferizer_destroy(MSBufferizer *obj){
	ms_bufferizer_uninit(obj);
	free(obj);
}

/* returns the number of bytes available in the bufferizer*/
__inline int ms_bufferizer_get_avail(MSBufferizer *obj)
{
	return obj->size;
}

#endif