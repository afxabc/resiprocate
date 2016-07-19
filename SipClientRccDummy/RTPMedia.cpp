
#include "RTPMedia.h"
#include "rutil\Lock.hxx"

RTPMedia::RTPMedia(IRTPMediaCallback* cb, const char* ip, unsigned short port, unsigned char payload, unsigned int rate)
	: cb_(cb)
	, rtpIP_(ip)
	, rtpPort_(port)
	, rtpPayload_(payload)
	, rtpRate_(rate)
	, remoteRtpPort_(0)
	, remoteRtpPayload_(payload)
	, remoteRtpRate_(rate)
{
	mShutdown = true;
}

RTPMedia::~RTPMedia()
{
	stop();
}

void RTPMedia::setRemote(const std::string& ip, unsigned short port, unsigned char payload, unsigned int rate)
{
	remoteRtpIP_ = ip;
	remoteRtpPort_ = port;
	remoteRtpPayload_ = payload;
	remoteRtpRate_ = rate;
}

bool RTPMedia::start(unsigned int ptime)
{
	stop();

	if (remoteRtpPort_ == 0)
		return false;

	resip::Lock lock(mutex_);

	if (rtpSession_.Create(rtpPort_) < 0)
		return false;

	rtpSession_.AddDestination(ntohl(inet_addr(remoteRtpIP_.c_str())), remoteRtpPort_);
	rtpSession_.SetDefaultPayloadType(remoteRtpPayload_);
	rtpSession_.SetTimestampUnit(1.0 / remoteRtpRate_);
	rtpSession_.SetDefaultTimeStampIncrement(ptime*remoteRtpRate_ / 1000);
	rtpSession_.SetDefaultMark(false);

	this->run();

	return true;
}

void RTPMedia::stop()
{
	if (mShutdown)
		return;

	mShutdown = true;
	this->shutdown();
	this->join();

	remoteRtpPort_ = 0;
	resip::Lock lock(mutex_);
	rtpSession_.Destroy();
}

int RTPMedia::sendData(char * data, int len)
{
	resip::Lock lock(mutex_);
	return rtpSession_.SendPacket(data, len);
}

unsigned short RTPMedia::tryPort(unsigned short port)
{
	rtpPort_ = port;

	while (rtpSession_.Create(rtpPort_) < 0)
		rtpPort_ += 2;
	rtpSession_.Destroy();

	return rtpPort_;
}

void RTPMedia::thread()
{
	fd_set fdset;
	struct timeval rttprev = { 0,0 }, rtt, tv;
	SOCKET sock1, sock2;
	rtpSession_.GetRTPSocket(&sock1);
	rtpSession_.GetRTCPSocket(&sock2);

	mShutdown = false;
	while (!isShutdown())
	{
		tv.tv_sec = 0;
		tv.tv_usec = 100 * 1000;
		FD_ZERO(&fdset);
		FD_SET(sock1, &fdset);
		FD_SET(sock2, &fdset);
		FD_SET(0, &fdset); // check for keypress

		select(FD_SETSIZE, &fdset, NULL, NULL, &tv);
		if (!FD_ISSET(0, &fdset))
			continue;

		resip::Lock lock(mutex_);

		rtpSession_.PollData();
		// check incoming packets
		if (rtpSession_.GotoFirstSourceWithData())
		{
			do
			{
				RTPSourceData *srcdat;
				srcdat = rtpSession_.GetCurrentSourceInfo();
				rtt = srcdat->INF_GetRoundTripTime();
				RTPPacket *pack;
				while ((pack = rtpSession_.GetNextPacket()) != NULL)
				{
					// You can examine the data here
					char* data = (char*)pack->GetPayload();
					int len = pack->GetPayloadLength();
					unsigned char payload = pack->GetPayloadType();

					if (payload == rtpPayload_ && cb_)
						cb_->onMediaData(data, len, payload);
					
					delete pack;
				}
			} while (rtpSession_.GotoNextSourceWithData());
		}
	}

}

typedef struct _telephone_event
{
#ifdef RTP_BIG_ENDIAN
	uint32_t evt : 8;
	uint32_t E : 1;
	uint32_t R : 1;
	uint32_t volume : 6;
	uint32_t duration : 16;
#else
	unsigned __int32 evt : 8;
	unsigned __int32 volume : 6;
	unsigned __int32 R : 1;
	unsigned __int32 E : 1;
	unsigned __int32 duration : 16;
#endif
}telephone_event_t;

#define TELEPHONE_EVENT 101
int RTPMedia::sendDtmfKey(int evt, int vol, int duration)
{
	telephone_event_t event_hdr;
	event_hdr.evt = evt;
	event_hdr.R = 0;
	event_hdr.E = 0;
	event_hdr.volume = vol;
	event_hdr.duration = duration;

	int ret = -1;
	do
	{
		if (rtpSession_.SendPacket((void *)(&event_hdr), sizeof(event_hdr), (unsigned char)TELEPHONE_EVENT, true, (unsigned long)0) < 0)
			break;

		event_hdr.duration += duration;
		if (rtpSession_.SendPacket((void *)(&event_hdr), sizeof(event_hdr), (unsigned char)TELEPHONE_EVENT, false, (unsigned long)0) < 0)
			break;

		event_hdr.duration += duration;
		event_hdr.E = 1;
		if (rtpSession_.SendPacket((void *)(&event_hdr), sizeof(event_hdr), (unsigned char)TELEPHONE_EVENT, false, (unsigned long)event_hdr.duration) < 0)
			break;

		ret = duration;
	} while (0);
	
	return ret;
}
