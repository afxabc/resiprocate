/*********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: audiostream
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\audiostream.h
*********************************************************************/


#ifndef MyAudioStream_H 

#define MyAudioStream_H 

#include <string> 
#include "mediastream/audiostream.h"
// dependency msticker 


using namespace resip;

class AppSession;
//----------------------------------------------------------------------------
// audiostream.h                                                                  
//----------------------------------------------------------------------------

//## class audiostream 
class MyAudioStream: public AudioStream
{

////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    MyAudioStream(AppSession *apps);
    
    //## auto_generated 
    virtual ~MyAudioStream();

	virtual MSTicker *getTicker();

public:
//	MSTicker *ticker;
	AppSession *app;

};


#endif  
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\audiostream.h
*********************************************************************/

