/*********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: audiostream
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\audiostream.h
*********************************************************************/


#ifndef MyVideoStream_H 

#define MyVideoStream_H 

#include <string> 

#include "videostream/videostream.h"

//## package Default 
#define MAX_RTP_SIZE	1500

//----------------------------------------------------------------------------
// audiostream.h                                                                  
//----------------------------------------------------------------------------
class AppSession;

//## class audiostream 
class MyVideoStream: public VideoStream
{

////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    MyVideoStream(AppSession *apps);
    
    //## auto_generated 
    virtual ~MyVideoStream();

#ifdef GUIDISPLAY

	void run();
	virtual void startpaint_message();
	virtual void stoppaint_message();
	virtual void paintbuf_message(int fmt, int w, int h, uint8_t *buf, int len);

#endif

public:

	AppSession *app;

};


#endif  
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\audiostream.h
*********************************************************************/

