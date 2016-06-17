/************************************************************************
*************************************************************************
*
* MODULE:  TNSYSLIB.H
* VERSION: V600_0,0112190904mfg_all.cuskchen
* $Revision:   1.2  $ $Modtime:   Aug 11 2008 16:42:56  $
* DESCRIPTION: LIBRARY DEFINITIONS FOR FUNCTION-CALL STACK
*
*************************************************************************
****copyright_c**********************************************************
                                   
                         COPYRIGHT STATEMENT

                        Copyright (c) 1991-2001
                           by Telenetworks
                          
All rights reserved. Copying, compilation, modification, distribution
or any other use whatsoever of this material is strictly prohibited
except in accordance with a Software License Agreement with
Telenetworks.

****copyright_c**********************************************************
****history**************************************************************
****history**************************************************************
****post-release-history*************************************************
$Log:   E:/vmdb/archives/TTC_V3/inc/tnsyslib.h-arc  $
 * 
 *    Rev 1.2   Aug 25 2008 14:14:42   zhangj
 * YHT/WTF/YDB DEBUG AT 20080825_1413

   Rev 1.69.2.0   26 Jul 2001 14:25:10   LIB_ADMIN-KC
Version 6.00 branched 07/26/2001

****post-release-history*************************************************
*************************************************************************

GENERAL NOTES

This file declares the Telenetworks system interface library for
protocol software.  The intent of system interface library is to
provide portablity for protocol software.

The library declares many objects (data types, macros and functions)
that can be used by portable protocol software.  Below is a list of
these objects.  They are fully documented in the Telenetworks System
Interface Specification.

PORTABLITY

For portablity, it is important to only reference the objects defined
by the Telenetworks System Interface Specification.  These objects
are listed below, and in the interface specification.

This file declares many objects in addition the the interface
specification, but these are not to be directly referenced.  Any
direct reference to these additional objects will reduce the
portablity of the software.

As an example, the BUF_CURS data type is part of the system interface
specification.  In most implementations, BUF_CURS will probably be
declared as a structure.  However, the fields of that stucture are
not part of the interface specification, and reference to them will
reduce portablity because the definition of the structure will
probably be different from one implementation to another.  All
manipulation of the BUF_CURS object should be done through the
BCUR_XXXXX macros.

To put the portablity rule simply, if the name of an object is not in
the following list, portable code should not access it.

   BUFFER MANAGEMENT INTERFACE
   ------------------------------------------
   BUF_ID               type defined for a buffer
   BUF_CURS             type defined for buffer manipulation cursor
   BUF_Q                type defined for queue of buffers
   BCUR_ADJUST()        adjust cursor to a new position
   BCUR_BUF()           returns buffer managed by a buffer cursor
   BCUR_ERROR()         checks for errors during use of a buffer cursor
   BCUR_GET_CHAR()      get character from buffer using cursor
   BCUR_GET_DATA()      get array of characters from buffer using cursor
   BCUR_GET_INT16()     extract 2-byte integer from buffer using cursor
   BCUR_GET_INT24()     extract 3-byte integer from buffer using cursor
   BCUR_GET_INT32()     extract 4-byte integer from buffer using cursor
   BCUR_GET_PTR()       returns pointer to data at cursor location
   BCUR_PUT_CHAR()      put character into buffer using cursor
   BCUR_PUT_DATA()      put array of characters into buffer using cursor
   BCUR_PUT_INT16()     store 2-byte integer into buffer using cursor
   BCUR_PUT_INT24()     store 3-byte integer into buffer using cursor
   BCUR_PUT_INT32()     store 4-byte integer into buffer using cursor
   BCUR_SEG             returns current segment managed by buffer cursor
   BCUR_SET()           initialize cursor to beginning of buffer
   BCUR_SET_OFFSET()    initialize cursor to arbitrary position in buffer
   BUFQ_APPEND()        add buffer to end of buffer queue
   BUFQ_FIRST()         get first buffer in a queue
   BUFQ_FREE()          release all buffers in a queue
   BUFQ_INIT()          initialize a buffer queue
   BUFQ_JOIN()          appends contents of one queue to another, clears original
   BUFQ_NEXT()          get next buffer in a queue
   BUFQ_PREPEND()       add buffer to beginning of buffer queue
   BUFQ_REMOVE()        remove buffer from a queue
   BUF_COPY()           make an actual (physical) copy of buffer
   BUF_COPY_ID()        make a logical copy of buffer
   BUF_FREE()           release buffer allocation
   BUF_GET_DATA_LEN()   get number of bytes of useful data in buffer
   BUF_GET_HEAD()       get initial number of bytes
   BUF_GET_TAIL_LEN()   get number of unused bytes at end of buffer
   BUF_JOIN()           join one buffer to end of another
   BUF_MAKE_CONT()      make initial length contig
   BUF_PAD_HEAD()       prepend space to beginning of useful buffer data
   BUF_PAD_TAIL()       append space to end of useful buffer data
   BUF_PUT_HEAD()       prepend data to head of buffer
   BUF_SPLIT()          break one buffer into two
   BUF_TRIM_HEAD()      remove space from beginning of useful buffer data
   BUF_TRIM_TAIL()      remove space from end of useful buffer data
   BSEG_ALLOC2()        allocate segment, read & write set with extra trailer
   BSEG_ALLOCATE()      allocate segment, read & write set to begining
   BSEG_FREE()          free a buffer segment
   BSEG_GET_DATA_LEN()  get length of data in buffer
   BSEG_GET_HI_ADDR()   get address of segments last storage byte
   BSEG_GET_LO_ADDR()   get address of segments first storage byte
   BSEG_GET_NEXT_SEG()  get pointer to next segment in chain
   BSEG_GET_READ_PTR()  get read byte pointer
   BSEG_GET_WRITE_PTR() get write byte pointer
   BSEG_SET_LEN()       set length of segment data
   BSEG_SET_NEXT_SEG()  set next segment in chain
   BSEG_SET_READ_PTR()  set read byte pointer
   BSEG_SET_WRITE_PTR() set write byte pointer

   BSEG_GET_DATA_PTR()       get read byte pointer, type cast
   BSEG_GET_TRAIL_SPACE()    get the empty space of the current segment
   BSEG_ADVANCE_WRITE_PTR()  advance the write pointer, not past end of seg
   BSEG_ALLOC_PRE_PAD()      alloc and leave space at head

   BUFFER MANAGEMENT INTERFACE - FOR BACKWARDS COMPATIBLITY
   ---------------------------------------------------------------
   BSEG_ALLOC()         like BSEG_ALLOC2
   BSEG_GET_ADDR()      like BSEG_GET_READ_PTR
   BSEG_GET_LEN()       like BSEG_GET_DATA_LEN
   BSEG_GET_SIZE()      not used
   BSEG_NEXT()          like BSEG_GET_NEXT_SEG
   BUF_GET_HEAD_LEN()   not used
   BUF_GET_LEN()        like BUF_GET_DATA_LEN

   MEMORY MANAGEMENT INTERFACE
   ------------------------------------------
   MEM_ALIGN()          align a pointer to allowed struct boundary
   MEM_ALLOC()          allocate a block of memory from heap
   MEM_CHR()            locate character in memory range
   MEM_COMP()           compare byte-by-byte two ranges of memory
   MEM_COPY()           copy range of memory to another location
   MEM_FREE()           deallocate a block of memory
   MEM_SET()            set all bytes of range of memory to value
   MEM_ZERO()           set all bytes of range of memory to zero


*************************************************************************
*************************************************************************
*/

#ifndef SYSLIB_H
#define SYSLIB_H

/*
** Include system header files
*/
//#include "Gendef.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef unsigned char BYTE;
/*
** The following section detects the environment, and disables
** default definitions for some facilities.
*/

/*typedef unsigned char BYTE;*/
/*
** Vector library errror return codes
*/
#define TNBPRI_LO 0
/*
** Streams like message data structure
** --- DO NOT REFERENCE ---
*/

typedef struct buf_dblk_s
{
   unsigned char *db_base;
   unsigned char *db_lim;
   unsigned char  db_ref;
}
BUF_DBLK_S;

/*
** Streams like message block structure
** --- DO NOT REFERENCE ---
*/

typedef struct buf_mblk_s
{
   struct buf_mblk_s *b_next;
   struct buf_mblk_s *b_prev;
   struct buf_mblk_s *b_cont;
   unsigned char     *b_wptr;
   unsigned char     *b_rptr;
   BUF_DBLK_S        *b_datap;
}
BUF_MBLK_S;

/*
** Buffer id data type
*/

typedef BUF_MBLK_S *BUF_ID;

/*
** Buffer cursor data type
*/

typedef struct buf_curs
{
   BUF_ID topseg;                      /* pointer to the first segment */
   BUF_ID curseg;                    /* pointer to the current segment */
   unsigned char *curptr;           /* current byte in current segment */
}
BUF_CURS;

/*
** Buffer queue data type
*/

typedef struct buf_q
{
   BUF_ID head;
   BUF_ID tail;
}
BUF_Q;


/*
** Buffer cursor macros.
*/

#define BCUR_ADJUST(curs, offset) \
   bcur_adjust ((curs),(offset))

#define BCUR_BUF(curs) \
   ((curs)->topseg)

#define BCUR_ERROR(curs) \
   ((curs)->curseg == NULL)

#define BCUR_GET_CHAR(curs, chr_p) { \
   if ((curs)->curseg != NULL) { \
      if ((curs)->curptr < (curs)->curseg->b_wptr) \
         *(chr_p) = *((curs)->curptr++); \
      else \
         bcur_get_char ((curs),(chr_p)); \
      } \
   }

#define BCUR_GET_DATA(curs, data, size, result_p) \
   (*(result_p) = bcur_get_data ((curs), (data), (size)))

#define BCUR_GET_INT16(curs, value_p) { \
   unsigned char temp; \
   BCUR_GET_CHAR ((curs), &temp); \
   *(value_p) = temp << 8; \
   BCUR_GET_CHAR ((curs), &temp); \
   *(value_p) |= temp; \
   }

#define BCUR_GET_INT24(curs, value_p, result_p) \
   (*(result_p) = bcur_get_long ((curs), (value_p), 3))

#define BCUR_GET_INT32(curs, value_p, result_p) \
   (*(result_p) = bcur_get_long ((curs), (value_p), 4))

#define BCUR_GET_PTR(curs) \
   ((curs)->curptr)

#define BCUR_PUT_CHAR(curs, chr) { \
   if ((curs)->curseg != NULL) { \
      if ((curs)->curptr < (curs)->curseg->b_wptr) \
         *((curs)->curptr++) = (chr); \
      else \
         bcur_put_char ((curs),(chr)); \
      } \
   }

#define BCUR_PUT_DATA(curs, data, len, result_p) \
   (*(result_p) = bcur_put_data ((curs), (data), (len)))

#define BCUR_SEG(curs) \
   ((curs)->curseg)

#define BCUR_SET(curs, buf) { \
   (curs)->topseg = (curs)->curseg = (buf); \
   (curs)->curptr = (buf)->b_rptr; \
   }

#define BCUR_SET_OFFSET(curs, buf, offset) \
   bcur_set_offset(curs, buf, offset)


/*
** Buffer queue macros
*/

#ifdef INLINE_BUFQ_MACROS

#define BUFQ_APPEND(bufq, buf) { \
(buf)->b_prev = (bufq)->tail; \
if ((bufq)->head == NULL) \
   (bufq)->head = (buf); \
else \
   (bufq)->tail->b_next = (buf); \
(bufq)->tail = (buf); \
(buf)->b_next = NULL; \
}

#else

#define BUFQ_APPEND(bufq, buf) \
   bufq_append ((bufq), (buf))

#endif

#define BUFQ_FIRST(bufq) \
   ((bufq)->head)

#define BUFQ_JOIN(q1,q2) \
   bufq_join(q1,q2)

#define BUFQ_FREE(bufq) \
   bufq_free (bufq)

#define BUFQ_INIT(bufq) \
   ((bufq)->head = (bufq)->tail = NULL)

#define BUFQ_MOVE(srcq, dstq) { \
(dstq)->head = (srcq)->head; \
(dstq)->tail = (srcq)->tail; \
(srcq)->head = (srcq)->tail = NULL; \
}

#define BUFQ_NEXT(buf) \
   ((buf)->b_next)

#ifdef INLINE_BUFQ_MACROS

#define BUFQ_PREPEND(bufq, buf) { \
(buf)->b_next = (bufq)->head; \
if ((bufq)->head == NULL) \
   (bufq)->tail = (buf); \
else \
   (bufq)->head->b_prev = (buf); \
(bufq)->head = (buf); \
(buf)->b_prev = NULL; \
}

#define BUFQ_REMOVE(bufq, buf_p) { \
if ((*(buf_p) = (bufq)->head) != NULL) { \
   if (((bufq)->head = (*(buf_p))->b_next) == NULL) \
      (bufq)->tail = NULL; \
   else \
      (bufq)->head->b_prev = NULL; \
   (*(buf_p))->b_next = (*(buf_p))->b_prev = NULL; \
}}

#else

#define BUFQ_PREPEND(bufq, buf) \
   bufq_prepend ((bufq), (buf))

#define BUFQ_REMOVE(bufq, buf_p) \
   (*(buf_p) = bufq_remove (bufq))

#endif

/*
** Buffer chain macros.
*/

#define BUF_COPY(buf, newb_p) \
   (*(newb_p) = copymsgbuf (buf))

#define BUF_COPY_ID(buf, newb_p) \
   (*(newb_p) = dupmsgbuf (buf))

#define BUF_FREE(buf) \
   freemsgbuf (buf)

#define BUF_GET_DATA_LEN(buf) \
   buf_get_len(buf)

#define BUF_GET_HEAD(bufp,data,len) \
   buf_get_head (bufp,data,len)

#define BUF_GET_TAIL_LEN(buf, len_p) \
   (*(len_p) = buf_get_tail_len (buf))

#define BUF_JOIN(buf1, buf2) \
   linkbuf ((buf1), (buf2))

#define BUF_MAKE_CONT(bufp,len) \
   buf_make_cont(bufp,len)

#define BUF_PAD_HEAD(bufp,len,result_p) \
   (*(result_p) = buf_put_head (bufp,NULL,len))

#define BUF_PAD_TAIL(buf, size, result_p) \
   (*(result_p) = buf_pad_tail ((buf), (size)))

#define BUF_PUT_HEAD(bufp,data,len) \
   buf_put_head (bufp,data,len)

#define BUF_SPLIT(buf, len, new_p) \
   (*(new_p) = buf_split ((buf), (len)))

#define BUF_TRIM_HEAD(bufp,len) \
   buf_get_head (bufp,NULL,len)

#define BUF_TRIM_TAIL(buf, size) \
   buf_trim_tail (buf, size)


/*
** Buffer segment macros.
*/
#define BSEG_ALLOCATE(size) \
   allocbuf(size,TNBPRI_LO)

#define BSEG_FREE(buf) \
   freebuf(buf)

#define BSEG_GET_DATA_LEN(buf) \
   ((int)((buf)->b_wptr - (buf)->b_rptr))

#define BSEG_GET_HI_ADDR(buf) \
   ((buf)->b_datap->db_lim)

#define BSEG_GET_LO_ADDR(buf) \
   ((buf)->b_datap->db_base)

#define BSEG_GET_NEXT_SEG(buf) \
   ((buf)->b_cont)

#define BSEG_GET_READ_PTR(buf) \
   ((buf)->b_rptr)

#define BSEG_GET_WRITE_PTR(buf) \
   ((buf)->b_wptr)

#define BSEG_SET_NEXT_SEG(buf,cp) \
   ((buf)->b_cont = (cp))

#define BSEG_SET_READ_PTR(buf,ptr) \
   ((buf)->b_rptr = (ptr))

#define BSEG_SET_WRITE_PTR(buf,ptr) \
   ((buf)->b_wptr = (ptr))

#define BSEG_GET_DATA_PTR(buf, type) \
   ((type)((buf)->b_rptr))

#define BSEG_GET_TRAIL_SPACE(buf) \
   ((buf)->b_datap->db_lim - (buf)->b_wptr)

#define BSEG_ADVANCE_WRITE_PTR(buf, count) \
   if (((buf)->b_wptr + (count)) < (buf)->b_datap->db_lim) \
      (buf)->b_wptr += (count); \
   else \
      (buf)->b_wptr = (buf)->b_datap->db_lim; \

#define BSEG_ALLOC_PRE_PAD(buf_p, size, head) \
   if ((*(buf_p) = BSEG_ALLOCATE((size))) != NULL) \
      { \
      BSEG_SET_READ_PTR(*(buf_p), BSEG_GET_READ_PTR(*(buf_p)) + head); \
      BSEG_ADVANCE_WRITE_PTR(*(buf_p), head); \
      }

/*
** Additional buffer macros.
** These are defined in terms of other buffer macros, and should
** not have to be ported.
*/

#define BCUR_PUT_INT16(curs, value) \
   BCUR_PUT_CHAR (curs, (char) (((value) >>  8) & 0xFF)); \
   BCUR_PUT_CHAR (curs, (char) ((value) & 0xFF));

#define BCUR_PUT_INT24(curs, value) \
   BCUR_PUT_CHAR (curs, (char) (((value) >> 16) & 0xFF)); \
   BCUR_PUT_CHAR (curs, (char) (((value) >>  8) & 0xFF)); \
   BCUR_PUT_CHAR (curs, (char) ((value) & 0xFF));

#define BCUR_PUT_INT32(curs, value) \
   BCUR_PUT_CHAR (curs, (char) (((value) >> 24) & 0xFF)); \
   BCUR_PUT_CHAR (curs, (char) (((value) >> 16) & 0xFF)); \
   BCUR_PUT_CHAR (curs, (char) (((value) >>  8) & 0xFF)); \
   BCUR_PUT_CHAR (curs, (char) ((value) & 0xFF));

#define BUF_GET_LEN(buf, len_p) \
   (*(len_p) = BUF_GET_DATA_LEN(buf))

#define BSEG_ALLOC2(buf_p, size, head, tail) \
   if ((*(buf_p) = BSEG_ALLOCATE((head)+(size)+(tail))) != NULL) { \
      BSEG_SET_WRITE_PTR (*(buf_p), BSEG_GET_WRITE_PTR (*(buf_p)) + (head) + (size)); \
      BSEG_SET_READ_PTR (*(buf_p), BSEG_GET_READ_PTR (*(buf_p)) + (head)); \
      }

#define BSEG_ALLOC(buf_p, size, head) \
   if ((*(buf_p) = BSEG_ALLOCATE((head)+(size))) != NULL) { \
      BSEG_SET_WRITE_PTR (*(buf_p), BSEG_GET_WRITE_PTR (*(buf_p)) + (head) + (size)); \
      BSEG_SET_READ_PTR (*(buf_p), BSEG_GET_READ_PTR (*(buf_p)) + (head)); \
      }

#define BSEG_GET_ADDR(buf, addr_p) \
   (*(addr_p) = BSEG_GET_READ_PTR(buf))

#define BSEG_GET_LEN(buf, len_p) \
   (*(len_p) = BSEG_GET_DATA_LEN(buf))

#define BSEG_SET_LEN(buf, len) \
   BSEG_SET_WRITE_PTR(buf,BSEG_GET_READ_PTR(buf)+len)

/*
** Buffer macros for backwards compatibility;
** Don't use these; there are better alternatives.
*/

#define BUF_GET_HEAD_LEN(buf, len_p) \
   (*(len_p) = (int) (BSEG_GET_READ_PTR(buf)-BSEG_GET_LO_ADDR(buf))

#define BSEG_GET_SIZE(buf, size_p) \
   (*(size_p) = (int) (BSEG_GET_HI_ADDR(buf)-BSEG_GET_READ_PTR(buf)))

#define BSEG_NEXT(buf, next_p) \
   (*(next_p) = BSEG_GET_NEXT_SEG(buf))


/*                    CUSTOMIZE HERE
*  The following MACROs are used by Telenetworks Device Drivers in
*  situations wherein the Device Driver designer wants to prevent
*  nested interrupts for some period of time and then allow interrupts
*  after the critical stage of ISR processing.
*
*  The purpose of using a MACRO is to make that Deviec Driver more
*  portable to various Operating Systems running on various CPUs.
*
*  If you are using a Telenetworks Device Driver you must provide,
*  under your OS, and for your CPU, a function that will disable
*  interrupts and enable interrupts as shown below.
*
*  If you are not using a Telenetworks Device Driver and have created
*  your own Device Driver you may also wish to use this technique to
*  make your driver more portable. If you do not wish to use this
*  system then there is no need to port these MACROs.
*
*/

/* The following macros are used to notify the O/S that an ISR
*  has been entered and exited.
*/

/*
** Memory management macros
*/

#define MEM_ALLOC(SIZE)       malloc(SIZE)

#define MEM_CHR(MEM,CHR,LEN)  memchr(MEM,CHR,LEN)
#define MEM_COMP(B1,B2,LEN)   memcmp(B1,B2,LEN)
#define MEM_COPY(DST,SRC,LEN) memcpy(DST,SRC,LEN)
#define MEM_FREE(MEM)         free(MEM)
#define MEM_SET(MEM,CHR,LEN)  memset(MEM,CHR,LEN)
#define MEM_ZERO(MEM,SIZE)    memset(MEM,0,SIZE)

/*
** MEM_ALIGN is used to ensure that pointers to structs and longs
** will align at the proper boundary for the system bus. In the vast
** majority of cases, this happens when the lowest two bits of the
** pointer are zero, as is done below. Otherwise, this macro requires
** a change for the system.
*/
#define MEM_ALIGN(PTR)        (void*)(((unsigned long)(PTR)+3)&(~0x3L))

/*
** System I/O routines
*/

/*
** Internal support routines for buffer management primitives
** --- DO NOT REFERENCE ---
*/

int bcur_adjust (BUF_CURS *curs, int offset);
void bcur_get_char (BUF_CURS *curs, BYTE *data);
int bcur_get_data (BUF_CURS *curs, BYTE *data, int size);
int bcur_get_long (BUF_CURS *curs, unsigned long *value, int size);
void bcur_put_char (BUF_CURS *curs, BYTE chr);
int bcur_put_data (BUF_CURS *curs, BYTE *data, int size);
int bcur_set_offset (BUF_CURS *curs, BUF_ID buf, int offset);
#ifndef INLINE_BUFQ_MACROS
void bufq_append (BUF_Q *bufq, BUF_ID buf);
#endif
void bufq_join (BUF_Q *q1, BUF_Q *q2);
int bufq_free (BUF_Q *bufq);
#ifndef INLINE_BUFQ_MACROS
void bufq_prepend (BUF_Q *bufq, BUF_ID buf);
BUF_ID bufq_remove (BUF_Q *bufq);
#endif
int buf_get_head (BUF_ID *bufp, BYTE *data, int len);
int buf_get_len (BUF_ID buf);
int buf_get_tail_len (BUF_ID buf);
int buf_make_cont (BUF_ID *bufp, int len);
int buf_pad_tail (BUF_ID buf, int len);
int buf_put_head (BUF_ID *bufp, BYTE *data, int len);
BUF_ID buf_split (BUF_ID buf, int size);
void buf_trim_tail (BUF_ID buf, int len);

/*
** Internal support routines for memory routines
** --- DO NOT REFERENCE ---
*/

#ifndef USE_STD_HEAP
#ifndef USE_MMLIB2
#ifndef F_MEM_TRACK_OWNER
void *mem_alloc (long size);
#endif
void mem_copy (char *dst, char *src, unsigned len);
int mem_comp (char *b1, char *b2, unsigned len);
char *mem_chr (char *mem, int chr, unsigned len);
void mem_free (char *mem);
void mem_set (char *mem, int chr, unsigned len);
#endif
#endif

#ifndef STREAMS_KERN

/*
** Streams like functions associated with buffers
** --- DO NOT REFERENCE ---
*/

BUF_MBLK_S *allocbuf (int size, int pri);
BUF_MBLK_S *copybuf (BUF_MBLK_S *oldp);
BUF_MBLK_S *copymsgbuf (BUF_MBLK_S *old);
BUF_MBLK_S *dupbuf (BUF_MBLK_S *old);
BUF_MBLK_S *dupmsgbuf (BUF_MBLK_S *old);
void freebuf (BUF_MBLK_S *buf);
void freemsgbuf (BUF_MBLK_S *buf);
void linkbuf (BUF_MBLK_S *buf1, BUF_MBLK_S *buf2);
BUF_MBLK_S *unlinkbuf (BUF_MBLK_S *old);

#endif                                                 /* STREAMS_KERN */

/*
** Timer interface support routines.
** --- DO NOT REFERENCE ---
*/
/*
** Timer system interface.
** Only system code should reference, not apps or protocol layers.
** --- DO NOT REFERENCE ---
*/

/*
** Timer system interface.
** Only test code should reference, not apps or protocol layers.
*/

#if defined (TMR_LIB_SIM)
PUBLIC void tmr_startSim();
PUBLIC void tmr_stopSim();
#endif


/*
** Interface for ISR signaling.
** --- DO NOT REFERENCE ---
*/

/*
** Signaling system interface.
** --- DO NOT REFERENCE ---
*/

/*
** Interrupt vector management functions
*/

/*
** General library functions
*/

void sys_break (void);
int  sys_getchr (void);
int  sys_getstr (char *str, int max_len);
void sys_io_init (void);
void sys_io_term (void);
void sys_putchr (int chr);
void sys_putstr (char *str);

#ifdef WIN_VXD
/* Fatal error handler definition.
*/
void Fatal_Halt(char * file, int line, char * str);
#endif


/* This is a common SAP for the timer library interface, and is used
** by several TN OSs, including PKERN and MTEX.
*/


/* This is a common SAP for the signal library interface, and is used
** by several TN OSs, including PKERN and MTEX.
*/


/* Variable to disable timer expiration during test conditions.
*/


#ifdef MTEX
/*
** Timer task semaphore
** --- DO NOT REFERENCE ---
*/
extern SEM TmrSem;
#endif                                                         /* MTEX */

#ifdef USE_MMLIB2
#include "mmlib2.h"
#endif                                                   /* USE_MMLIB2 */

#endif                                                     /* SYSLIB_H */
