/*********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: PayloadType
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\PayloadType.h
*********************************************************************/

#ifndef RTPPROFILE_H

#define RTPPROFILE_H

#include "rutil/Data.hxx"
#include "str_utils.h"
//----------------------------------------------------------------------------
// PayloadType.h                                                                  
//----------------------------------------------------------------------------
using namespace resip;
//## package Default 

#define PAYLOAD_AUDIO_CONTINUOUS 0
#define PAYLOAD_AUDIO_PACKETIZED 1
#define PAYLOAD_VIDEO 2
#define PAYLOAD_OTHER 3 

#define PROFILE_MAX_PAYLOADS 128
/*
** Streams like message data structure
** --- DO NOT REFERENCE ---
*/
class PayloadType
{
public:
	PayloadType(int load,int rate,char sample,int len,int bitrate,Data mine,int chan):
	      type(load),
		  clock_rate(rate),
		  bits_per_sample(sample),
		  pattern_length(len),
		  normal_bitrate(bitrate),
		  mime_type(mine),
		  channels(chan),
		  recv_fmtp(),
		  send_fmtp(),
		  flags(0),
		  user_data()
	{}
	~PayloadType()
	{}

	static bool fmtp_get_value(const char *fmtp, const char *param_name, char *result, size_t result_len);

	void payloadtype_set_recv_fmtp(Data fmtp)
	{
		recv_fmtp = fmtp;
	}

	/*** Sets a send parameters (fmtp) for the PayloadType.
	* This method is provided for applications using RTP with SDP, but
	* actually the ftmp information is not used for RTP processing.
	**/
	void payloadtype_set_send_fmtp(Data fmtp)
	{
		send_fmtp = fmtp;
	}

public:
	int type; /**< one of PAYLOAD_* macros*/
	int clock_rate; /**< rtp clock rate*/
	char bits_per_sample;	/* in case of continuous audio data */
	Data zero_pattern;
	int pattern_length;
	/* other useful information for the application*/
	int normal_bitrate;	/*in bit/s */
	Data mime_type; /**<actually the submime, ex: pcm, pcma, gsm*/
	int channels; /**< number of channels of audio */
	Data recv_fmtp; /* various format parameters for the incoming stream */
	Data send_fmtp; /* various format parameters for the outgoing stream */
	int flags;
	Data user_data;
};

inline bool PayloadType::fmtp_get_value(const char *fmtp, const char *param_name, char *result, size_t result_len)
{
	const char *pos=strstr(fmtp,param_name);
	if (pos)
	{
		const char *equal=strchr(pos,'=');
		if (equal)
		{
			int copied;
			const char *end=strchr(equal+1,';');
			if (end==NULL)
			{
				end=fmtp+strlen(fmtp); /*assuming this is the last param */
			}
			copied=MIN(result_len-1,end-(equal+1));
			strncpy(result,equal+1,copied);
			result[copied]='\0';
			return true;
		}
	}
	return false;
}


class RtpProfile
{
public:
	RtpProfile(Data mm):name(mm)
	{
		for(int i=0; i<PROFILE_MAX_PAYLOADS; i++)
			payload[i] = NULL;
	}

	void rtpprofile_set_payload(int idx, PayloadType *pt)
	{
		if (idx<0 || idx>=PROFILE_MAX_PAYLOADS) 
		{
//			cout<<"Bad index "<<idx<<endl;
			return;
		}
		payload[idx] = pt;
	}
	PayloadType * rtpprofile_get_payload(int idx)
	{
		if (idx<0 || idx>=PROFILE_MAX_PAYLOADS)
		{
			return NULL;
		}
		return payload[idx];
	}
	int rtpprofile_get_payload_number_from_mime(const char *mime)
	{
		PayloadType *pt;
		int i;
		for (i=0;i<PROFILE_MAX_PAYLOADS;i++)
		{
			pt=rtpprofile_get_payload(i);
			if (pt!=NULL)
			{
				if (strcasecmp(pt->mime_type.c_str(),mime)==0)
				{
					return i;
				}
			}
		}
		return -1;
	}


public:
	Data name;
	PayloadType *payload[PROFILE_MAX_PAYLOADS];
};

#endif      /* RTPPROFILE_H */
