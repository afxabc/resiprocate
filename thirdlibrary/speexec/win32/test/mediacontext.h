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
#define PCMU    0
#define G723    4
#define PCMA    8
#define G729    18
#define H263    34
#define H263_1998    98

#define WINSND_NBUFS 10
#define WINSND_OUT_NBUFS 20
#define WINSND_NSAMPLES 160
#define WINSND_MINIMUMBUFFER 5
#define WINSND_OUT_DELAY 0.02
//using namespace resip;

class MediaContext
{
   public:
	   MediaContext() {} // baboons
	   virtual ~MediaContext(){}

//      unsigned long GetTimeStamp() 	{ return timestamp; }
//	  int GetPayloadLength() 		{ return payloaddata.size(); }
//	  Data& GetPayload() { return payload; }
//	  unsigned char GetPayloadType()	{ return payloadtype; }
//	  bool IsMarked()					{ return marked; }

//	  void SetPayload(Fifo<Data>& data) { payload=data; }
//	  void SetPayloadType(unsigned char type)	{ payloadtype=type; }
 
public:
//	unsigned long timestamp;
//	bool marked;
	queue_t payload0; // payload data
	queue_t payload1;
//	unsigned char payloadtype;
 
};


#endif

/* ====================================================================
 * 
 * ====================================================================
 * 
 *
 */
