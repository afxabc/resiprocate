#ifndef FileWrite_H 

#define FileWrite_H 
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

#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"


//----------------------------------------------------------------------------
// fileplayer.h                                                                  
//----------------------------------------------------------------------------

//## package Default 


//## class fileplayer 
class FileWrite: public Processor
{


////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    FileWrite(int val = 20);
    
    //## auto_generated 
    virtual ~FileWrite();


////    Operations    ////
public :
    
	virtual void process(MediaContext &mc);
    int filesave(resip::Data name) ;
	int filesave2(resip::Data name) ;

	WAVEFORMATEX wfx;
	queue_t payloadRef;
	queue_t payloadDat;
//	resip::Data rb;
};


#endif  