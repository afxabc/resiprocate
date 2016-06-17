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
#include "MyAudioStream.hxx"
#include "AppSession.hxx"
#include "DialInstance.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

//----------------------------------------------------------------------------
// audiostream.cpp                                                                  
//----------------------------------------------------------------------------

MyAudioStream::MyAudioStream(AppSession *apps):
	app(apps)
{
}

MyAudioStream::~MyAudioStream()
{
	InfoLog(<<"delete AudioStream ");
}

MSTicker *MyAudioStream::getTicker()
{
	return app->getParent()->getAudioTicker();
}

/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\audiostream.cpp
*********************************************************************/

