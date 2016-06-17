/************************************************************************
*************************************************************************
*
* MODULE:  BUFLIB.C
* VERSION: V600_0,0112190904mfg_all.cuskchen
* $Revision:   1.37.1.0  $ $Modtime:   26 Jul 2001 12:17:52  $
* DESCRIPTION: DATA BUFFER LIBRARY
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
    $Log:   S:/dev/admin/lib/buflib.c  $

   Rev 1.37.1.0   26 Jul 2001 14:12:50   LIB_ADMIN-KC
Version 6.00 branched 07/26/2001

****post-release-history*************************************************
*************************************************************************

GENERAL NOTES

This file implements a buffer manipulation library, compatible with
streams, but portable to other systems.

*************************************************************************
*************************************************************************
*/
#include "buflib.h"


/* Define the minimum length of buffer segment when appending
*  or prepending data via functions buf_pad_tail or buf_put_head.
*  This prevents lots of small data segments from being created.
*
*  A sufficiently large value was chosen to handle worst case
*  protocol encapsulation (for instance X.25 over Frame Relay).
*/
#ifndef MIN_HEADER
#define MIN_HEADER 32
#endif

/* The default allocation for buflib uses the standard syslib MEM_ALLOC
*  and MEM_FREE. However many systems must allocate protocol buffers
*  from special pools. In order to allow for system customization
*  without changes to the buflib.c source, the following macros are
*  defined. By default, they map to MEM_ALLOC/FREE. However they can
*  be overridded in tune.h to map to a system-specific allocator.
*  BUFLIB_MBLK_ALLOC/FREE are macros used to allocate the message block
*  associated with a buffer segment. BUFLIB_DATA_ALLOC/FREE are macros
*  used to allocate the data block and associated physical data that
*  are associated with a buffer segment.
*/

#ifndef BUFLIB_MBLK_ALLOC
#define BUFLIB_MBLK_ALLOC(size)      MEM_ALLOC(size)
#endif
#ifndef BUFLIB_MBLK_FREE
#define BUFLIB_MBLK_FREE(mem)        MEM_FREE(mem)
#endif
#ifndef BUFLIB_DATA_ALLOC
#define BUFLIB_DATA_ALLOC(size)      MEM_ALLOC(size)
#endif
#ifndef BUFLIB_DATA_FREE
#define BUFLIB_DATA_FREE(mem)        MEM_FREE(mem)
#endif

/*
* This function moves adjusts the cursor by some offset from its current
* position, without adding or removing data. The function returns 0 for
* success, -1 for failure.
*
*/
int bcur_adjust (
   BUF_CURS *curs,
   int offset
   )
{
   BUF_ID buf = curs->curseg;
   BYTE * curptr = curs->curptr;
   int len;

   if (offset > 0)
      {
      for (;;)
         {
         if (buf == NULL)
            {
            curs->curseg = NULL;
            return -1;
            }
         if (offset <= (len = (int) ((BYTE *)buf->b_wptr - curptr)))
            break;
         offset -= len;
         if ((buf = buf->b_cont) != NULL)
            curptr = buf->b_rptr;
         }
      curs->curseg = buf;
      curs->curptr = curptr + offset;
      }

   else if (offset < 0)
      {
      offset = -offset;
      if (offset <= (len = (int) (curptr - (BYTE *)buf->b_rptr)))
         {
         curs->curptr = curptr - offset;
         }
      else
         {
         int n;
         BUF_ID tmp;

         /* Get total size of all segments up to the current one.
         */
         for (len = 0, tmp = curs->topseg;
              (tmp != NULL) && (tmp->b_cont != buf); tmp = tmp->b_cont)
            len += (int) (tmp->b_wptr - tmp->b_rptr);
         len += (int) (curptr - (BYTE *)tmp->b_rptr);

         /* Locate the new cursor segment.
         */
         len -= offset;
         if (len < 0)
            {
            curs->curseg = NULL;
            return -1;
            }
         for (tmp = curs->topseg;
              len > (n = (int) (tmp->b_wptr - tmp->b_rptr));
              len -= n, tmp = tmp->b_cont)
            ;
         curs->curseg = tmp;
         curs->curptr = tmp->b_rptr + len;
         }
      }

   return 0;
}


void bcur_get_char (
   BUF_CURS *curs,
   BYTE *chr
   )
{
   if (curs->curseg == NULL)
      return;

   while (curs->curptr >= curs->curseg->b_wptr)
      {
      if ((curs->curseg = curs->curseg->b_cont) == NULL)
         return;
      curs->curptr = curs->curseg->b_rptr;
      }

   *chr = *curs->curptr++;
}


int bcur_get_data (
   BUF_CURS *curs,
   BYTE *data,
   int len
   )
{
   register BYTE *ptr;
   register int avail;

   if (curs->curseg == NULL)
      return -1;

   ptr = data;

   for (;;)
      {
      while ((avail = (int) (curs->curseg->b_wptr - curs->curptr)) <= 0)
         {
         if ((curs->curseg = curs->curseg->b_cont) == NULL)
            return (int) (ptr - data);
         curs->curptr = curs->curseg->b_rptr;
         }

      if (avail >= len)
         break;

      MEM_COPY (ptr, curs->curptr, avail);
      curs->curptr += avail;
      ptr += avail;
      len -= avail;
      }

   MEM_COPY (ptr, curs->curptr, len);
   curs->curptr += len;
   ptr += len;

   return (int) (ptr - data);
}

int bcur_get_long (
   BUF_CURS *curs,
   unsigned long *value,
   int len
   )
{
   BYTE chr;

   if (len > sizeof (unsigned long))
      return -1;

   *value = 0;
   while (len-- != 0)
      {
      BCUR_GET_CHAR (curs, &chr);
      *value = (*value << 8) | (unsigned long) chr;
      }
   return (curs->curseg == NULL) ? -1 : len;
}


void bcur_put_char (
   BUF_CURS *curs,
   BYTE chr
   )
{
   if (curs->curseg == NULL)
      return;

   while (curs->curptr >= curs->curseg->b_wptr)
      {
      if ((curs->curseg = curs->curseg->b_cont) == NULL)
         return;
      curs->curptr = curs->curseg->b_rptr;
      }

   *curs->curptr++ = chr;
}


int bcur_put_data (
   BUF_CURS *curs,
   BYTE *data,
   int len
   )
{
   register BYTE *ptr;
   register int avail;

   if (curs->curseg == NULL)
      return -1;

   ptr = data;

   for (;;)
      {
      while ((avail = (int) (curs->curseg->b_wptr - curs->curptr)) <= 0)
         {
         if ((curs->curseg = curs->curseg->b_cont) == NULL)
            return (int) (ptr - data);
         curs->curptr = curs->curseg->b_rptr;
         }

      if (avail >= len)
         break;

      MEM_COPY (curs->curptr, ptr, avail);
      curs->curptr += avail;
      ptr += avail;
      len -= avail;
      }

   MEM_COPY (curs->curptr, ptr, len);
   curs->curptr += len;
   ptr += len;

   return (int) (ptr - data);
}

/*
* This function sets the buffer curso to an intermediate position in the
* buffer, specified by the offset parameter.  The function returns 0 for
* success, -1 for failure.
*
*/
int bcur_set_offset (
   BUF_CURS *curs,
   BUF_ID buf,
   int offset
   )
{
   register int len;

   curs->topseg = buf;

   for (;;)
      {
      if (buf == NULL)
         {
         curs->curseg = NULL;
         return -1;
         }
      if (offset <= (len = (int) (buf->b_wptr - buf->b_rptr)))
         break;
      offset -= len;
      buf = buf->b_cont;
      }

   curs->curseg = buf;
   curs->curptr = buf->b_rptr + offset;

   return 0;
}

#ifndef INLINE_BUFQ_MACROS

void bufq_append (
   BUF_Q *bufq,
   BUF_ID buf
   )
{
   buf->b_prev = bufq->tail;
   if (bufq->head == NULL)
      bufq->head = buf;
   else
      bufq->tail->b_next = buf;
   bufq->tail = buf;
   buf->b_next = NULL;
}
#endif

/*
* Free all entries in a buffer queue.  Returns the number of
* buffers that were freed.
*
*/
int bufq_free (
   BUF_Q *bufq
   )
{
   BUF_ID buf;
   int i;

   for (i = 0;; ++i)
      {
      BUFQ_REMOVE (bufq, &buf);
      if (buf == NULL)
         break;
      BUF_FREE (buf);
      }

   return i;
}

/*
* Joins (appends) one q2 to the end of q1
* Either queue can be empty.
* q2 is zeroed.
*
*/
void bufq_join (
   BUF_Q *q1,
   BUF_Q *q2
   )
{
   if (q1->head == NULL)
      {
      q1->head = q2->head;
      q1->tail = q2->tail;
      }
   else if (q2->head != NULL)
      {
      q2->head->b_prev = q1->tail;
      q1->tail->b_next = q2->head;
      q1->tail = q2->tail;
      }

   q2->head = q2->tail = NULL;
}

#ifndef INLINE_BUFQ_MACROS

void bufq_prepend (
   BUF_Q *bufq,
   BUF_ID buf
   )
{
   buf->b_next = bufq->head;
   if (bufq->head == NULL)
      bufq->tail = buf;
   else
      bufq->head->b_prev = buf;
   bufq->head = buf;
   buf->b_prev = NULL;
}
#endif

#ifndef INLINE_BUFQ_MACROS

BUF_ID bufq_remove (
   BUF_Q *bufq
   )
{
   BUF_ID buf;

   if ((buf = bufq->head) != NULL)
      {
      if ((bufq->head = buf->b_next) == NULL)
         bufq->tail = NULL;
      else
         bufq->head->b_prev = NULL;
      buf->b_next = buf->b_prev = NULL;
      }

   return buf;
}
#endif

/*
* This function copies data from the head of a buffer.
*
* If the specified amount of data to be copied is greater than or
* equal to the amount of data in the buffer, the buffer is not freed,
* and is left with a zero length.
*
* If the specified amount of data to be copied is less than the
* amount of data in the buffer, the buffer's length is reduced
* by the amount copied.
*
* The function returns the number of bytes copied.
*
*/
int buf_get_head (
   BUF_ID *bufp,
   BYTE *data,
   int len
   )
{
   BUF_ID buf, tmp;
   int avail;
   int i;

   i = 0;
   buf = *bufp;

   while (buf != NULL && len > 0)
      {
      /* Compute how much data is in this segment
      */
      avail = (int) (buf->b_wptr - buf->b_rptr);

      /* Check there is more than enough data in this segment.
      */
      if (avail > len)
         {
         /* There is more data than required.
         *  Copy it, adjust segment length, then quit.
         */
         if (data != NULL)
            MEM_COPY (data + i, buf->b_rptr, len);
         buf->b_rptr += len;
         i += len;
         break;
         }
      else
         {
         /* Not quite enough data - or exactly enough.
         *  Copy everything from this segment.
         */
         if (data != NULL)
            MEM_COPY (data + i, buf->b_rptr, avail);
         buf->b_rptr += avail;
         i += avail;

         /* Compute how much more data is needed.
         */
         len -= avail;

         /* We copied all of this segment.
         *  Point to the next one.  Quit if there are no more
         *  segments (this prevents us from freeing the last
         *  segment and leaves it in an empty state).
         */
         if ((tmp = unlinkbuf (buf)) == NULL)
            break;

         /* There was a next segment: dump the current one.
         */
         freebuf (buf);
         buf = tmp;
         }
      }

   /* Update user's buffer pointer
   */
   *bufp = buf;

   /* Return amount of data read.
   */
   return i;
}


int buf_get_len (
   BUF_ID buf
   )
{
   register int size;

   for (size = 0; buf != NULL; buf = buf->b_cont)
      size += (int) (buf->b_wptr - buf->b_rptr);

   return size;
}

/*
* This function returns the number of free bytes available at the end
* of the buffer (the last segment).
*/
int buf_get_tail_len (
   BUF_ID buf
   )
{
   BUF_ID end;

   /* Find the last segment
   */
   for (end = buf; end->b_cont != NULL; end = end->b_cont)
      ;

   /* return available space in last segment
   */
   return ((int) (end->b_datap->db_lim - end->b_wptr));
}

/*
* Force the first 'len' bytes of a buffer to be in contiguous storage.
* Returns -1 on failure.  Returns 0 to len, inclusive, if successfull.
* A return value less than 'len' occurs when there is less than 'len'
* bytes of storage in the buffer.
*
* If succesful, the bufp is set to point to the new buffer storage,
* and the old value of bufp should not be used.
*
*/
int buf_make_cont (
   BUF_ID *bufp,
   int len
   )
{
   BUF_ID newp;
   int result;

   /* Trival case; the buffer is contiguous for the requested size.
   */
   if (BSEG_GET_DATA_LEN (*bufp) >= len)
      {
      result = len;
      }

   /* The first segment of the buffer was not long enough.
   ** Allocate one that is.
   */
   else if ((newp = BSEG_ALLOCATE (len)) == NULL)
      {
      result = -1;
      }

   /* Allocation succeeded.  Now for the hard part...
   */
   else
      {
      /* Select the lesser of:
      ** 1) the length of the data in the original buffer.
      ** 2) the requested size.
      */
      if (len > (result = BUF_GET_DATA_LEN (*bufp)))
         len = result;

      /* Copy the data from the original into the new buffer,
      ** then update the write pointer of the new buffer.
      */
      BUF_GET_HEAD (bufp, BSEG_GET_READ_PTR (newp), len);
      BSEG_SET_WRITE_PTR (newp, BSEG_GET_WRITE_PTR (newp) + len);

      /* If there is no data left in the original buffer, free it.
      ** Otherwise, join it to the end of the new buffer.
      */
      if (BSEG_GET_DATA_LEN (*bufp) == 0)
         BSEG_FREE (*bufp);
      else
         BUF_JOIN (newp, *bufp);

      /* Update caller's buffer pointer.
      */
      *bufp = newp;
      }

   return result;
}

/*
* This function appends space to a buffer.
*
* The function returns 0 if the space was succesfully appended, and
* -1 if there was a failure (memory shortage).
*
*/
int buf_pad_tail (
   BUF_ID buf,
   int len
   )
{
   BUF_ID end, tmp;
   int avail;

   /* Find the last segment
   */
   for (end = buf; end->b_cont != NULL; end = end->b_cont)
      ;

   /* Compute available space in last segment
   */
   avail = (int) (end->b_datap->db_lim - end->b_wptr);

   /* If there is not enough space in the last segment,
   *  then we need to add another.
   */
   if (len > avail)
      {
      /* Reduce size of required space by the amount of space that
      *  is already available.  Then try to allocate a new segment
      *  large enough to hold the rest of the data.
      */
      len -= avail;
      if ((tmp = allocbuf (len, TNBPRI_LO)) == NULL)
         return -1;
#ifdef STREAMS_KERN
      tmp->b_datap->db_type = M_DATA;
#endif

      /* Success. Adjust last segment's write pointer.
      */
      end->b_wptr += avail;

      /* Make new segment the last segment.
      */
      linkbuf (end, tmp);
      end = tmp;
      }

   /* Adjust length of tail.
   */
   end->b_wptr += len;

   return 0;
}

/*
* This function copies data into a buffer.  The data is prepended to
* the head of the buffer.
*
* The function returns 0 if the data was succesfully appended, and
* -1 if the data could not be appended because of memory shortage.
*
*/
int buf_put_head (
   BUF_ID *bufp,
   BYTE *data,
   int len
   )
{
   BUF_ID buf, tmp;
   int avail;

   buf = *bufp;

   /* Compute available space in first segment.
   */
      {
      avail = (int) (buf->b_rptr - buf->b_datap->db_base);
      }

   /* if there is not enough space in the first segment,
   *  then we need to add another.
   */
   if (len > avail)
      {
      /* Reduce size of required space by the amount of space that
      *  is already available.  Then try to allocate a new segment
      *  large enough to hold the rest of the data (plus a little
      *  extra room for additional protocol headers).
      */
      len -= avail;
      tmp = allocbuf ((len > MIN_HEADER) ? len : MIN_HEADER, TNBPRI_LO);
      if (tmp == NULL)
         return -1;
#ifdef STREAMS_KERN
      tmp->b_datap->db_type = M_DATA;
#endif

      /* Success.  Adjust first segment's read pointer and
      *  then copy some data into the available space.
      */
      if (avail > 0)
         {
         buf->b_rptr -= avail;
         if (data != NULL)
            MEM_COPY (buf->b_rptr, data + len, avail);
         }

      /* Make new segment the first segment, with it's read and write
      *  pointers pointing to the end of segment.
      */
      tmp->b_wptr = tmp->b_datap->db_lim;
      tmp->b_rptr = tmp->b_datap->db_lim;
      linkbuf (tmp, buf);
      buf = tmp;
      }

   /* Copy data into buffer segment.
   */
   buf->b_rptr -= len;
   if (data != NULL)
      MEM_COPY (buf->b_rptr, data, len);
   *bufp = buf;

   return 0;
}

BUF_ID buf_split (
   BUF_ID buf,
   int size
   )
{
   BUF_ID newp;
   int avail;

   newp = NULL;

   while (buf != NULL)
      {
      avail = (int) (buf->b_wptr - buf->b_rptr);
      if (size == avail)
         {
         newp = buf->b_cont;
         buf->b_cont = NULL;
         break;
         }
      else if (size < avail)
         {
         if ((newp = dupbuf (buf)) != NULL)
            {
            newp->b_cont = buf->b_cont;
            buf->b_cont = NULL;
            newp->b_rptr += size;
            buf->b_wptr -= avail - size;
            }
         break;
         }
      size -= avail;
      buf = buf->b_cont;
      }

   return newp;
}

void buf_trim_tail (
   BUF_ID buf,
   int size
   )
{
   BUF_ID tmp;
   int n;

   /* Get total size of all segments
   */
   for (n = 0, tmp = buf; tmp != NULL; tmp = tmp->b_cont)
      n += (int) (tmp->b_wptr - tmp->b_rptr);

   while (buf->b_cont != NULL)
      {
      /* Calculate size of tail
      */
      n -= (int) (buf->b_wptr - buf->b_rptr);

      /* if size of tail is smaller than amount to trim,
      *  discard the tail and quit
      */
      if (n <= size)
         {
         freemsgbuf (buf->b_cont);
         buf->b_cont = NULL;
         if ((size -= n) < 0)
            size = 0;
         break;
         }

      /* goto next segment
      */
      buf = buf->b_cont;
      }

   /* trim first segment
   */
   if (size < (int) (buf->b_wptr - buf->b_rptr))
      buf->b_wptr -= size;
   else
      buf->b_wptr = buf->b_rptr;
}

#ifndef STREAMS_KERN

BUF_MBLK_S *allocbuf (
   int size,
   int pri
   )
{
   BYTE *buff;
   BUF_DBLK_S *dblk;
   BUF_MBLK_S *mblk;

/*   NOT_USED (pri);*/

   if ((mblk = (BUF_MBLK_S *) BUFLIB_MBLK_ALLOC (sizeof (BUF_MBLK_S))) == NULL)
      {
      return NULL;
      }

   if ((dblk = (BUF_DBLK_S *) BUFLIB_DATA_ALLOC (sizeof (BUF_DBLK_S) + size)) == NULL)
      {
      BUFLIB_MBLK_FREE (mblk);
      return NULL;
      }

   buff = (BYTE *) (dblk + 1);

   dblk->db_base = buff;
   dblk->db_lim = buff + size;
   dblk->db_ref = 1;

   mblk->b_next = NULL;
   mblk->b_prev = NULL;
   mblk->b_cont = NULL;
   mblk->b_wptr = buff;
   mblk->b_rptr = buff;
   mblk->b_datap = dblk;

   return mblk;
}

/*
* Makes an actual copy of a message segment.
*/
BUF_MBLK_S * copybuf
  (BUF_MBLK_S *          oldp)
{
   int               len = (int) (oldp->b_wptr - oldp->b_rptr);
   BUF_MBLK_S *          newp;

   newp = allocbuf ((int) (oldp->b_datap->db_lim - oldp->b_datap->db_base),
      TNBPRI_LO);
   if (newp == NULL)
      {
      return NULL;
      }
   newp->b_rptr = newp->b_datap->db_base
      + (int) (oldp->b_rptr - oldp->b_datap->db_base);
   newp->b_wptr = newp->b_rptr + len;
   MEM_COPY (newp->b_rptr, oldp->b_rptr, len);

   return newp;
}


/*
* Makes an actual copy of a message. Any segmentation is preserved.
*/
BUF_MBLK_S * copymsgbuf
  (BUF_MBLK_S *          oldp)
{
   BUF_MBLK_S *          newp;

   if ((newp = copybuf (oldp)) != NULL)
      {
      BUF_MBLK_S *p1 = oldp;
      BUF_MBLK_S *p2 = newp;

      while (p1->b_cont != NULL)
         {
         if ((p2->b_cont = copybuf (p1->b_cont)) == NULL)
            {
            freemsgbuf (newp);
            newp = NULL;
            break;
            }
         p1 = p1->b_cont;
         p2 = p2->b_cont;
         }
      }

   return newp;
}


BUF_MBLK_S *dupbuf (
   BUF_MBLK_S *old
   )
{
   BUF_MBLK_S *newp;

   if ((newp = (BUF_MBLK_S *) BUFLIB_MBLK_ALLOC (sizeof (BUF_MBLK_S))) == NULL)
      {
      return NULL;
      }

   old->b_datap->db_ref++;

   newp->b_next    = NULL;
   newp->b_prev    = NULL;
   newp->b_cont    = NULL;
   newp->b_wptr    = old->b_wptr;
   newp->b_rptr    = old->b_rptr;
   newp->b_datap   = old->b_datap;

   return newp;
}


BUF_MBLK_S *dupmsgbuf (
   BUF_MBLK_S *old
   )
{
   BUF_MBLK_S *newp;

   if ((newp = dupbuf (old)) != NULL)
      {
      BUF_MBLK_S *p1 = old;
      BUF_MBLK_S *p2 = newp;

      while (p1->b_cont != NULL)
         {
         if ((p2->b_cont = dupbuf (p1->b_cont)) == NULL)
            {
            freemsgbuf (newp);
            newp = NULL;
            break;
            }
         p1 = p1->b_cont;
         p2 = p2->b_cont;
         }
      }

   return newp;
}


void freebuf (
   BUF_MBLK_S *buf
   )
{
   if (buf->b_datap->db_ref == 0)
      {
      printf("\nfreebuf: bad db_ref value");
      }
   if (--buf->b_datap->db_ref == 0)
      {
      BUFLIB_DATA_FREE (buf->b_datap);
      }
   BUFLIB_MBLK_FREE (buf);
}


void freemsgbuf (
   BUF_MBLK_S *buf
   )
{
   BUF_MBLK_S *tmp;

   while (buf != NULL)
      {
      tmp = buf->b_cont;
      freebuf (buf);
      buf = tmp;
      }
}


void linkbuf (
   BUF_MBLK_S *buf1,
   BUF_MBLK_S *buf2
   )
{
   while (buf1->b_cont != NULL)
      buf1 = buf1->b_cont;
   buf1->b_cont = buf2;
}


BUF_MBLK_S *unlinkbuf (
   BUF_MBLK_S *old
   )
{
   BUF_MBLK_S *newp;

   newp = old->b_cont;
   old->b_cont = NULL;

   return newp;
}

#endif                                                 /* STREAMS_KERN */


