/*
*/
#include "rtpread.h"
#include <iostream>
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace resip;
using namespace std;

RtpRead::RtpRead(RTPSession* session):counter(0)
{
	sess = session;
}

RtpRead::~RtpRead()
{
}

void RtpRead::process(MediaContext &mc)
{
	sess->PollData();

	// check incoming packets
	if (sess->GotoFirstSourceWithData())
	{
		do
		{
			RTPPacket *pack;

			while ((pack = sess->GetNextPacket()) != NULL)
			{
				// You can examine the data here
				mblk_t *buf=allocb(pack->GetPayloadLength(),0);
				memcpy(buf->b_rptr,pack->GetPayload(),pack->GetPayloadLength());
				buf->b_wptr+=pack->GetPayloadLength();
				counter+=pack->GetPayloadLength();

				mblk_set_timestamp_info(buf,pack->GetTimeStamp());
				mblk_set_marker_info(buf,pack->IsMarked());
				mblk_set_payload_type(buf,pack->GetPayloadType());
				
				putq(&mc.payload0,buf);

				// we don't longer need the packet, so
				// we'll delete it
				delete pack;
			}
		}
		while(sess->GotoNextSourceWithData());
	}
	
}







