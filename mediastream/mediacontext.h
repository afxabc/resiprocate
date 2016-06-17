#if !defined(MediaContext_H)
#define MediaContext_H

//#include <vector>
//#include <iosfwd>
#include "str_utils.h"
//#include "ProcessorChain.h"

/*
0    PCMU        A            8,000       1
4    G723        A            8,000       1
8    PCMA        A            8,000       1
18   G729        A            8,000       1
31      H261        V           90,000
34      H263        V           90,000
dyn     H263-1998   V           90,000
*/

#define CodecNo_PCMU    0
#define CodecNo_GSM     3
#define CodecNo_G723    4
#define CodecNo_PCMA    8
#define CodecNo_G729    18
#define CodecNo_H263    34
#define CodecNo_iLBC    97
#define CodecNo_H263_1998  98

#define TELEPHONE_EVENT 101

#define WINSND_NBUFS 10
#define WINSND_OUT_NBUFS 20
#define WINSND_NSAMPLES 160
#define WINSND_MINIMUMBUFFER 5
#define WINSND_OUT_DELAY 0.02
#define JITTER 60
//using namespace resip;

class MediaContext
{
   public:
	   MediaContext() 
	   {
		   qinit(&payload0);
		   qinit(&payload1);
	   } // baboons

	   ~MediaContext()
	   {
		   flushq(&payload0, 0);
		   flushq(&payload1, 0);
	   }

 
public:
	int statisnum;
	int counter;
	queue_t payload0;
	queue_t payload1;
 
};


#endif

/* ====================================================================
 * 
 * ====================================================================
 * 
 *
 */
