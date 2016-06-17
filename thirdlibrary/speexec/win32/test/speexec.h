#ifndef SPEEXEC_H 
#define SPEEXEC_H 

#include "MediaContext.h"
#include "processor.h"
#include "str_utils.h"
#include <speex/speex_echo.h>
#include <speex/speex_preprocess.h>

typedef struct SpeexECState{
	SpeexEchoState *ecstate;
	MSBufferizer inRef;
    MSBufferizer inData;
	int framesize;
	int filterlength;
	int samplerate;
	SpeexPreprocessState *den;
        int ref;
        int echo;
        int out;
}SpeexECState;

typedef struct _MSCPoint{
	struct _MSFilter *filter;
	int pin;
} MSCPoint;

typedef struct _MSQueue
{
	queue_t q;
	MSCPoint prev;
	MSCPoint next;
}MSQueue;
struct _MSFilter{
	
	MSQueue **inputs;
	MSQueue **outputs;	
};

typedef struct _MSFilter MSFilter;

class SpeexEc: public Processor
{

////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    SpeexEc();
    
    //## auto_generated 
    virtual ~SpeexEc();

////    Operations    ////
public :

	virtual void process(MediaContext &mc);
    //## operation process(msfilter) 
//	void encode(resip::Data& data);

//	void decode(resip::Data& data);
private:
	int ms_bufferizer_get_avail(MSBufferizer *obj);
	int speex_ec_set_sr(void *arg);
	int speex_ec_set_framesize(void *arg);
	int speex_ec_set_filterlength(void *arg);
	void speex_ec_init();
	void speex_ec_uninit();

protected:
	unsigned char type;
	queue_t payload;
	SpeexECState s;

};


#endif  