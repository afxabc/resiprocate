#ifndef AudioRecord_H 

#define AudioRecord_H 
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include "str_utils.h"
#include <mmsystem.h>
#include "rutil/Mutex.hxx"
#include "MediaContext.h"
#include "processor.h"

#include <fstream>
#include <stdlib.h>
#include "waveheader.h"
#include "rutil/Data.hxx"

using namespace resip;

//----------------------------------------------------------------------------
// AudioRecord.h                                                                  
//----------------------------------------------------------------------------


//## class fileplayer 
class AudioRecord: public Processor
{


////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    AudioRecord();
    
    //## auto_generated 
    virtual ~AudioRecord();


////    Operations    ////
private:
	 bool filesave(resip::Data name);
public :
    
	virtual void process(MediaContext &mc);
	bool StartRecording(int nRecordVoice, bool bRecordCompress);
    bool StopRecording();
	bool SaveRecordingToWaveFile(resip::Data name);
	bool ResetRecording();

	WAVEFORMATEX wfx;
	queue_t payload;
	bool recordOn;
	int nRecordBytes;
	Data filename;
};


#endif  