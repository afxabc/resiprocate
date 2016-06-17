#ifndef AudioPlayer_H 

#define AudioPlayer_H 

#include <fstream>
#include <stdlib.h>
#include "waveheader.h"
#include "MediaContext.h"
#include "processor.h"
#include "rutil/Data.hxx"
#include "rutil/Mutex.hxx"
#include "rutil/Lock.hxx"
//----------------------------------------------------------------------------
// fileplayer.h                                                                  
//----------------------------------------------------------------------------
using namespace resip;
//## package Default 
class WincardWrite;


//## class fileplayer 
class AudioPlayer: public Processor
{


////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    AudioPlayer(int val = 20);
    
    //## auto_generated 
    virtual ~AudioPlayer();


////    Operations    ////
public :
    
	int Convert44kTo8k(unsigned short*in, int shLen, unsigned char*out);
	virtual void process(MediaContext &mc);
    //## operation initial(msfilter) 
	bool fileopen(resip::Data name);
//	int filesave(resip::Data name) ;

 
	bool LocalPlay(bool stat);
	bool PlayWaveOpen(Data sFileName);
    bool PlayWaveSkipTo(int nSeconds);
    int PlayWaveTotalTime();
    bool PlayWavePause();
    bool PlayWaveStart(bool bListen);
    bool PlayWaveStop();
    int PlayWavePosition();

    //## operation process(msfilter) 
//protected:
	std::fstream file;
	int codectype;
	int nAvgBytesPerSec;/*byte/s*/
	int wBitsPerSample;/*byte per sample*/
	int nSamplesPerSec;/*sample rate*/
	int nchannels;/*channel*/
	int hsize;
	int filelen;
	int interval;
	mblk_t *rb;

	bool playOn;
	bool localPlayOn;
	WincardWrite* localCardwrite;
	MediaContext* context;
//	resip::Data rb;
//	queue_t payloadDat;

	mutable resip::Mutex mMutex;
};


#endif  