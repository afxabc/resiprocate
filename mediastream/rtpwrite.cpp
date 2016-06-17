/*
*/
#include "rtpwrite.h"
#include <iostream>
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace std;
using namespace resip;

RtpWrite::RtpWrite(RTPSession* session):sess(session),
mark(false),
counter(0)
{
}

RtpWrite::~RtpWrite()
{
}

void RtpWrite::process(MediaContext &mc)
{
	mblk_t *im;

	while((im=getq(&mc.payload0))!=NULL)
	{
		unsigned long tampinc=mblk_get_timestamp_info(im);
		unsigned char type=mblk_get_payload_type(im);
		mark=mblk_get_marker_info(im);
		int len = msgdsize(im);
		msgpullup(im, -1);
		sess->SendPacket((void *)im->b_rptr,len,type,mark,tampinc);
		counter+=msgdsize(im);
		freemsg(im);

	}
}




