#ifndef Dtmf_H 

#define Dtmf_H 

#include "waveheader.h"
#include "jrtplib/rtpsession.h"
#include "str_utils.h"
#include "MediaContext.h"
#include "processor.h"
#include <math.h>

typedef struct _telephone_event
{
#ifdef RTP_BIG_ENDIAN
	uint32_t evt:8;
	uint32_t E:1;
	uint32_t R:1;
	uint32_t volume:6;
	uint32_t duration:16;
#else
	unsigned __int32 evt:8;
	unsigned __int32 volume:6;
	unsigned __int32 R:1;
	unsigned __int32 E:1;
	unsigned __int32 duration:16;
#endif
}telephone_event_t;

class Dtmf: public Processor  
{

////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    Dtmf();
    
    //## auto_generated 
    ~Dtmf();

	static int Dtmf_send(RTPSession *session, char dtmf);
	static mblk_t *Dtmf_create_event(unsigned __int8 evt, int end, unsigned __int8 volume, unsigned __int16 duration);
	void Dtmf_parse(mblk_t *m0, queue_t* payload);
	void process(MediaContext &mc);
	void Dtmf::notify_tev(telephone_event_t *evt, queue_t* payload);
	void Dtmf::notify_events_ended(telephone_event_t *events, int num, queue_t* payload);

public:
	mblk_t *current_tev;
};

#define TEV_DTMF_0			(0)
#define TEV_DTMF_1			(1)
#define TEV_DTMF_2			(2)
#define TEV_DTMF_3			(3)
#define TEV_DTMF_4			(4)
#define TEV_DTMF_5			(5)
#define TEV_DTMF_6			(6)
#define TEV_DTMF_7			(7)
#define TEV_DTMF_8			(8)
#define TEV_DTMF_9			(9)
#define TEV_DTMF_STAR		(10)
#define TEV_DTMF_POUND		(11)
#define TEV_DTMF_A			(12)
#define TEV_DTMF_B			(13)
#define TEV_DTMF_C			(14)
#define TEV_DTMF_D			(15)
#define TEV_FLASH			(16)



#define TELEPHONY_EVENTS_ALLOCATED_SIZE		(4*sizeof(telephone_event_t))

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif
#endif