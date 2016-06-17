/********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: audiostream
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\audiostream.cpp
*********************************************************************/
#include <iostream>
#include "MyVideoStream.hxx"
#include "AppSession.hxx"
#include "DialInstance.hxx"

#ifdef VIDEOTEST
#include "videofile.h"
#endif
#include "MediaContext.h"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

//----------------------------------------------------------------------------
// audiostream.cpp                                                                  
//----------------------------------------------------------------------------
using namespace std;
using namespace resip;
//## package Default 

MyVideoStream::MyVideoStream(AppSession *apps):
	app(apps)
{
}

MyVideoStream::~MyVideoStream()
{
	InfoLog(<<"delete AudioStream ");
}

#ifdef GUIDISPLAY

void MyVideoStream::startpaint_message()
{
	app->getParent()->msgStartPaint();
}

void MyVideoStream::stoppaint_message()
{
	app->getParent()->msgStopPaint();
}

/*MS_YUV420P=0,MS_YUYV,MS_RGB24,MS_MJPEG,MS_UYVY,MS_YUY2,*/
void MyVideoStream::paintbuf_message(int fmt, int w, int h, uint8_t *buf, int len)
{
	app->getParent()->msgPaintBuf(fmt, w, h, buf, len);
}


#endif /*GUIDISPLAY*/

/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\audiostream.cpp
*********************************************************************/

