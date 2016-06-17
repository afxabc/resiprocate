#include "speexec.h"

#ifdef HAVE_CONFIG_H
//#include "mediastreamer-config.h"
#endif

#ifdef WIN32
#include <malloc.h> /* for alloca */
#endif

static const int framesize=128;
static const int filter_length=2048; /*250 ms*/



SpeexEc::SpeexEc()
{
	speex_ec_init();
}

SpeexEc::~SpeexEc() 
{
	speex_ec_uninit();
}


/* returns the number of bytes available in the bufferizer*/
int SpeexEc::ms_bufferizer_get_avail(MSBufferizer *obj)
{
	return obj->size;
}
/********************************************************/


void SpeexEc::speex_ec_init()
{
	//SpeexECState *s=(SpeexECState *)ms_new(SpeexECState,1);
	//s=(SpeexECState *)malloc(sizeof(SpeexECState));

	s.samplerate=8000;
	s.framesize=framesize;
	s.filterlength=filter_length;

	ms_bufferizer_init(&s.inRef);
	ms_bufferizer_init(&s.inData);
	s.ecstate=speex_echo_state_init(s.framesize,s.filterlength);
	s.den = speex_preprocess_state_init(s.framesize, s.samplerate);

	mblk_t *mEmpty;
    mEmpty = allocb(framesize, 0);	
	memset(mEmpty->b_wptr, 0, framesize);	
	mEmpty->b_wptr += framesize;
	ms_bufferizer_put(&s.inRef, mEmpty);

//	f->data=s;
}

void SpeexEc::speex_ec_uninit()
{
//	SpeexECState *s=(SpeexECState*)f->data;
	ms_bufferizer_uninit(&s.inRef);
	ms_bufferizer_uninit(&s.inData);
	speex_echo_state_destroy(s.ecstate);
	if (s.den!=NULL)
	  speex_preprocess_state_destroy(s.den);

	//ms_free(s);
	//free(s);
}


/*	inputs[0]= reference signal (sent to soundcard)
	inputs[1]= echo signal	(read from soundcard)

    By contrary to the input[2]
	mc.payload0 = data;
	mc.payload1 = reference data;
*/
void SpeexEc::process(MediaContext &mc)
{
	int nbytes = s.framesize * 2;
	uint8_t *in1;
	mblk_t *om0,*om1;

#ifdef HAVE_SPEEX_NOISE
	spx_int32_t *noise=(spx_int32_t*)alloca(nbytes*sizeof(spx_int32_t)+1);
#else
	float *noise=NULL;
#endif
#ifdef AMD_WIN32_HACK
	static int count=0;
#endif

	/*read input and put in bufferizers; then clear payload*/
	ms_bufferizer_put_from_queue(&s.inData, &mc.payload0);
	ms_bufferizer_put_from_queue(&s.inRef, &mc.payload1);
	
	in1 = (uint8_t*)alloca(nbytes);

	while (ms_bufferizer_get_avail(&s.inData)>=nbytes && ms_bufferizer_get_avail(&s.inRef)>=nbytes)
	{
		om0 = allocb(nbytes, 0);
		ms_bufferizer_read(&s.inRef, (uint8_t*)om0->b_wptr, nbytes);
        om0->b_wptr += nbytes;
		/* we have reference signal */
		/* the reference signal is sent through outputs[0]*/
		//ms_queue_put(f->outputs[0],om0);???

		ms_bufferizer_read(&s.inData, in1, nbytes);
		/* we have echo signal */
		om1=allocb(nbytes, 0);
		speex_echo_cancel(s.ecstate,(short*)in1,(short*)om0->b_rptr,(short*)om1->b_wptr,(spx_int32_t*)noise);
		if (s.den!=NULL && noise!=NULL)
		  speex_preprocess(s.den, (short*)om1->b_wptr, (spx_int32_t*)noise);

		om1->b_wptr += nbytes;
		//ms_queue_put(mc.payload0,om1);
		putq(&mc.payload0, om1);

#ifdef AMD_WIN32_HACK
		count++;
		if (count==100*3)
		{
			ms_message("periodic reset of echo canceller.");
			speex_echo_state_reset(s.ecstate);
			count=0;
		}		
#endif
	}

	/*If the size of reference data is more than 4*320, clear &s.in[0] and &s.in[1] */
	if (ms_bufferizer_get_avail(&s.inRef)> 4*320*(s.samplerate/8000)) /* above 4*20ms -> useless */
	  {
	    /* reset evrything */
	    flushq(&s.inRef.q,0);
	    flushq(&s.inData.q,0);
	    ms_bufferizer_init(&s.inRef);
	    ms_bufferizer_init(&s.inData);
	    speex_echo_state_reset(s.ecstate);
	  }

	/*If the size of signal data is more than 4*320, read and then send, the rest still save in &s.in[1]*/
	while (ms_bufferizer_get_avail(&s.inData)> 4*320*(s.samplerate/8000))
	{
		om1=allocb(nbytes,0);
		ms_bufferizer_read(&s.inData,(uint8_t*)om1->b_wptr,nbytes);
		om1->b_wptr+=nbytes;
		/*too much echo signal, sending anyway*/
		putq(&mc.payload0,om1);
//		ms_queue_put(mc.payload0,om1);
		speex_echo_state_reset(s.ecstate);
	}
	
}

int SpeexEc::speex_ec_set_sr(void *arg)
{
#ifdef SPEEX_ECHO_SET_SAMPLING_RATE
	//SpeexECState *s=(SpeexECState*)f->data;

	s.samplerate = *(int*)arg;

	if (s.ecstate==NULL)
		speex_echo_state_destroy(s.ecstate);
	if (s.den!=NULL)
	  speex_preprocess_state_destroy(s.den);

	s.ecstate=speex_echo_state_init(s.framesize,s.filterlength);
	speex_echo_ctl(s.ecstate, SPEEX_ECHO_SET_SAMPLING_RATE, &s.samplerate);
	s.den = speex_preprocess_state_init(s.framesize, s.samplerate);
#else
	ms_error("Speex echocanceler does not support 16Khz sampling rate in this version!");
#endif
	return 0;
}

int SpeexEc::speex_ec_set_framesize(void *arg)
{
//	SpeexECState *s=(SpeexECState*)f->data;
	s.framesize = *(int*)arg;

	if (s.ecstate==NULL)
		speex_echo_state_destroy(s.ecstate);
	if (s.den!=NULL)
	  speex_preprocess_state_destroy(s.den);

	s.ecstate=speex_echo_state_init(s.framesize,s.filterlength);
#ifdef SPEEX_ECHO_SET_SAMPLING_RATE
	speex_echo_ctl(s.ecstate, SPEEX_ECHO_SET_SAMPLING_RATE, &s.samplerate);
#endif
	s.den = speex_preprocess_state_init(s.framesize, s.samplerate);
	return 0;
}

int SpeexEc::speex_ec_set_filterlength(void *arg)
{
	//SpeexECState *s=(SpeexECState*)f->data;
	s.filterlength = *(int*)arg;

	if (s.ecstate==NULL)
		speex_echo_state_destroy(s.ecstate);
	if (s.den!=NULL)
	  speex_preprocess_state_destroy(s.den);

	s.ecstate=speex_echo_state_init(s.framesize,s.filterlength);
#ifdef SPEEX_ECHO_SET_SAMPLING_RATE
	speex_echo_ctl(s.ecstate, SPEEX_ECHO_SET_SAMPLING_RATE, &s.samplerate);
#endif
	s.den = speex_preprocess_state_init(s.framesize, s.samplerate);

	return 0;
}