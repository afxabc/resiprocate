/********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: audiocode
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\AudioEncode.cpp
*********************************************************************/
#include <iostream>
#include "AudioEncode.h"
#include "rutil/Fifo.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

//----------------------------------------------------------------------------
// AudioEncode.cpp                                                                  
//----------------------------------------------------------------------------
using namespace std;
using namespace resip;
//## package Default 

//## class AudioEncode 


AudioEncode::AudioEncode(unsigned char pt):type(pt)
{
	switch(pt)
	{
	case CodecNo_PCMU:
	case CodecNo_PCMA:
		codec = new CodecPcm(pt);
		break;
	case CodecNo_GSM:
		codec = new CodecGsm();
		break;
	case CodecNo_iLBC:
		codec = new CodeciLBC();
		break;
	case CodecNo_G723:
		codec = new CodecG723();
		break;
	case CodecNo_G729:
		codec = new CodecG729();
		break;
	default:
		codec = new CodecPcm(CodecNo_PCMA);
		break;
	}
}

AudioEncode::~AudioEncode() 
{
	delete codec;
}

void AudioEncode::process(MediaContext &mc)
{
    //#[ operation process(msfilter) 
    //#]

//	ms_bufferizer_put_from_queue(&inData, &mc.payload0);
	codec->encode(&mc.payload0);
#if 0
		if(type == PCMU)
		{
			while(ms_bufferizer_get_avail(&inData)>=320)
			{
				mblk_t *inm = allocb(320, 0);
				ms_bufferizer_read(&inData, (uint8_t*)inm->b_wptr, 320);
				inm->b_wptr += 320;
			
				mblk_t *ulawbuf = allocb(160,0);
				for (int i=0;i<160;i++)
				{
					*ulawbuf->b_wptr=s16_to_ulaw(((__int16*)inm->b_rptr)[i]);
					ulawbuf->b_wptr++;
				}
				freemsg(inm);
				mblk_set_payload_type(ulawbuf,PCMU);
				mblk_set_timestamp_info(ulawbuf,160);
				putq(&mc.payload0,ulawbuf);
			}
		}
		else if(type == PCMA)
		{
			while(ms_bufferizer_get_avail(&inData)>=320)
			{
				mblk_t *inm = allocb(320, 0);
				ms_bufferizer_read(&inData, (uint8_t*)inm->b_wptr, 320);
				inm->b_wptr += 320;
			
				mblk_t *alawbuf = allocb(160,0);
				for (int i=0;i<160;i++)
				{
					*alawbuf->b_wptr=s16_to_alaw(((__int16*)inm->b_rptr)[i]);
					alawbuf->b_wptr++;
				}
				freemsg(inm);
				mblk_set_payload_type(alawbuf,PCMA);
				mblk_set_timestamp_info(alawbuf,160);
				putq(&mc.payload0,alawbuf);
			}
		}

		}
//		mc.SetPayloadType(type);
#endif
}


/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\fileplayer.cpp
*********************************************************************/
