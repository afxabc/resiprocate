#include "speexec.h"

#ifdef HAVE_CONFIG_H
//#include "mediastreamer-config.h"
#endif

#ifdef WIN32
#include <malloc.h> /* for alloca */
#endif

#define REAL_APP_SPEEXECHO


SpeexEc::SpeexEc(int len, int framesize, int filter_length)
{
	echo_flag = 0;
	emptyLen = len;   //27
	s.samplerate = 8000;
	s.framesize = framesize;
	s.filterlength = filter_length;
	speex_ec_init();	
	FillEmptyPackets();
}

SpeexEc::~SpeexEc() 
{
	speex_ec_uninit();
}

void SpeexEc::FillEmptyPackets()
{
	mblk_t *mEmpty;
	int len = 80;

	for (int i=0; i<emptyLen; i++)
	{
		mEmpty = allocb(len, 0);	
		memset(mEmpty->b_rptr, 0x55, len);	
		mEmpty->b_wptr += len;

#ifdef REAL_APP_SPEEXECHO
		ms_bufferizer_put(&s.inRef, mEmpty);
#else
	    ms_bufferizer_put(&s.inData, mEmpty);
#endif
	}
}

void SpeexEc::speex_ec_init()
{
	ms_bufferizer_init(&s.inRef);
	ms_bufferizer_init(&s.inData);
	s.ecstate=speex_echo_state_init(s.framesize, s.filterlength);
	s.den = speex_preprocess_state_init(s.framesize, s.samplerate);
	/*added at 20091027*/
	speex_echo_ctl(s.ecstate, SPEEX_ECHO_SET_SAMPLING_RATE, &s.samplerate);
    speex_preprocess_ctl(s.den , SPEEX_PREPROCESS_SET_ECHO_STATE, s.ecstate);

}

void SpeexEc::speex_ec_uninit()
{
	ms_bufferizer_uninit(&s.inRef);
	ms_bufferizer_uninit(&s.inData);
	speex_echo_state_destroy(s.ecstate);
	if (s.den!=NULL)
	  speex_preprocess_state_destroy(s.den);
}

/*added at 20091028*/
void SpeexEc::OpenEchoCancellation()
{
	echo_flag = 1;
}

void SpeexEc::CloseEchoCancellation()
{
	echo_flag = 0;
}


/***********************************************
mc.payload0 = echo signal	(read from soundcard)
mc.payload1 = reference signal (sent to soundcard)
*************************************************/
void SpeexEc::process(MediaContext &mc)
{
	int nbytes = s.framesize*2;
	uint8_t *mic;
	mblk_t *ref,*out;

#ifdef HAVE_SPEEX_NOISE
	spx_int32_t *noise=(spx_int32_t*)alloca(nbytes*sizeof(spx_int32_t)+1);
#else
	float *noise=NULL;
#endif

#ifdef AMD_WIN32_HACK
	static int count=0;
#endif

	if (echo_flag == 0)
	{
		return;
	}

	/*read input and put in bufferizers; then clear payload*/

#ifdef REAL_APP_SPEEXECHO
	ms_bufferizer_put_from_queue(&s.inRef, &mc.payload1);
#else

	mblk_t* inm=NULL;
	for (inm=qbegin(&mc.payload0); !qend(&mc.payload0,inm); inm=qnext(&mc.payload0,inm))
	{
        mblk_t *tmp;
		tmp = dupmsg(inm);
		ms_bufferizer_put(&s.inRef, tmp);
	}
#endif
	ms_bufferizer_put_from_queue(&s.inData, &mc.payload0);
	
	/*malloc in the stack, needn't free*/
	mic = (uint8_t*)alloca(nbytes);

	while (ms_bufferizer_get_avail(&s.inData)>=nbytes && ms_bufferizer_get_avail(&s.inRef)>=nbytes)
	{
		/* we have reference signal */
		ref = allocb(nbytes, 0);
		ms_bufferizer_read(&s.inRef, (uint8_t*)ref->b_wptr, nbytes);
        ref->b_wptr += nbytes;
		
		/* we have echo signal */
		ms_bufferizer_read(&s.inData, mic, nbytes);		
		out=allocb(nbytes, 0);
		/*speex_echo_cancel(s.ecstate,(short*)ref->b_rptr,(short*)in1,(short*)out->b_wptr,(spx_int32_t*)noise);
		if (s.den!=NULL && noise!=NULL)
		  speex_preprocess(s.den, (short*)out->b_wptr, (spx_int32_t*)noise);*/		
     
		speex_echo_cancellation(s.ecstate,  (short*)mic, (short*)ref->b_rptr, (short*)out->b_wptr);
        speex_preprocess_run(s.den, (short*)out->b_wptr);

		out->b_wptr += nbytes;

		putq(&mc.payload0, out);

		/*added at 20091028*/
		freemsg(ref);

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

	/*If the size of reference data is more than x*320, clear &s.inRef and &s.inData */
	if (ms_bufferizer_get_avail(&s.inRef)> 100*320*(s.samplerate/8000)) /* above 4*20ms -> useless */
	  {
	    /* reset evrything */
	    flushq(&s.inRef.q,0);
	    flushq(&s.inData.q,0);
	    ms_bufferizer_init(&s.inRef);
	    ms_bufferizer_init(&s.inData);
	    speex_echo_state_reset(s.ecstate);
		/*added at 20091028*/
		FillEmptyPackets();
	  }

	/*If the size of signal data is more than 4*320, read and then send, the rest still save in &s.in[1]*/
	while (ms_bufferizer_get_avail(&s.inData)> 4*320*(s.samplerate/8000))
	{
		out=allocb(nbytes,0);
		ms_bufferizer_read(&s.inData,(uint8_t*)out->b_wptr,nbytes);
		out->b_wptr+=nbytes;
		/*too much echo signal, sending anyway*/
		putq(&mc.payload0,out);
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