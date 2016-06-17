/********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: audiocode
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\AudioDecode.cpp
*********************************************************************/
#include <iostream>
#include "AudioDecode.h"
#include "rutil/Fifo.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

//----------------------------------------------------------------------------
// AudioDecode.cpp                                                                  
//----------------------------------------------------------------------------
using namespace std;
using namespace resip;
//## package Default 

//## class AudioDecode 


AudioDecode::AudioDecode(unsigned char pt):type(pt)
{
	codecpcma = new CodecPcm(CodecNo_PCMA);
	codecpcmu = new CodecPcm(CodecNo_PCMU);
	codecgsm = new CodecGsm();
	codecilbc = new CodeciLBC();
	codecg723 = new CodecG723();
	codecg729 = new CodecG729();
	qinit(&payload);
}

AudioDecode::~AudioDecode() 
{
	delete codecpcma;
	delete codecpcmu;
	delete codecgsm;
	delete codecilbc;
	delete codecg723;
	delete codecg729;
}

void AudioDecode::process(MediaContext &mc)
{
    //#[ operation process(msfilter) 
    //#]
	unsigned char type;
	
	mblk_t *inm;
	mblk_t *om;

	while(inm=getq(&mc.payload0))
	{
		putq(&payload,inm);
	}
	while(inm=getq(&payload))
	{
		type = mblk_get_payload_type(inm);
		switch(type)
		{
		case CodecNo_PCMA:
			om = codecpcma->decode(inm);
			break;
		case CodecNo_PCMU:
			om = codecpcmu->decode(inm);
			break;
		case CodecNo_GSM:
			om = codecgsm->decode(inm);
			break;
		case CodecNo_iLBC:
			om = codecilbc->decode(inm);
			break;
		case CodecNo_G723:
			om = codecg723->decode(inm);
			break;
		case CodecNo_G729:
			om = codecg729->decode(inm);
			break;
		case TELEPHONE_EVENT:
			om = dupmsg(inm);
			break;
		default:
			om = NULL;
			break;
		}
		freemsg(inm);
		if(om)
			putq(&mc.payload0, om);
	}
} 
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\fileplayer.cpp
*********************************************************************/
