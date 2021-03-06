
#include "RTPMedia.h"
#include "rutil\Lock.hxx"
#include "buffer.h"

#include "jrtplib3\rtpsessionparams.h"
#include "jrtplib3\rtpudpv4transmitter.h"
#include "jrtplib3\rtpipv4address.h"
#include "jrtplib3\rtptimeutilities.h"
#include "jrtplib3\rtppacket.h"


RTPMedia::RTPMedia(IRTPMediaCallback* cb, const char* ip, unsigned short port, unsigned char payload, unsigned int rate)
	: cb_(cb)
	, rtpIP_(ip)
	, rtpPort_(port)
	, rtpPayload_(payload)
	, rtpRate_(rate)
	, remoteRtpPort_(0)
	, remoteRtpPayload_(payload)
	, remoteRtpRate_(rate)
	, hdrextID_(1)
{
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

void RTPMedia::setRemoteSelf()
{
	remoteRtpIP_ = rtpIP_;
	remoteRtpPort_ = rtpPort_;
	remoteRtpPayload_ = rtpPayload_;
	remoteRtpRate_ = rtpRate_;
}

bool RTPMedia::start(unsigned int ptime, int fps)
{
	stop();

	if (remoteRtpPort_ == 0)
		return false;

	RTPSessionParams params;
	if (fps == 0)
		params.SetOwnTimestampUnit(1.0 / remoteRtpRate_);
	else params.SetOwnTimestampUnit(1.0 / fps);
	params.SetAcceptOwnPackets(true);
//	params.SetMaximumPacketSize(64000);

	RTPUDPv4TransmissionParams trans;
	trans.SetPortbase(rtpPort_);
	trans.SetBindIP(ntohl(inet_addr(rtpIP_.c_str())));
	trans.SetRTPSendBuffer(4 * 1024 * 1024);
	trans.SetRTPReceiveBuffer(4 * 1024 * 1024);

	int ret = Create(params, &trans);
	if (ret < 0)
		return false;

	RTPIPv4Address addr(ntohl(inet_addr(remoteRtpIP_.c_str())), remoteRtpPort_);
	AddDestination(addr);
	SetDefaultPayloadType(remoteRtpPayload_);
	if (fps == 0)
		SetDefaultTimestampIncrement(ptime*remoteRtpRate_ / 1000);
	else SetDefaultTimestampIncrement(1000 / fps);
	SetDefaultMark(false);

//	this->run();

	return true;
}

void RTPMedia::stop()
{
	if (this->IsActive())
	{
		Destroy();
		remoteRtpPort_ = 0;
	}
}

int RTPMedia::sendData(char * data, int len)
{
	int ret = SendPacket(data, len);
	return ret;
}

int RTPMedia::sendData(char * data, int len, unsigned char pt, bool mark, unsigned long timestampinc)
{
	int ret = SendPacket(data, len, pt, mark, timestampinc);
	return ret;
}

void RTPMedia::OnRTPPacket(RTPPacket * pack, const RTPTime & receivetime, const RTPAddress * senderaddress)
{
	char* data = (char*)pack->GetPayloadData();
	int len = pack->GetPayloadLength();
	unsigned char payload = pack->GetPayloadType();

	cb_->onMediaData(data, len, payload, ntohs(pack->GetSequenceNumber()) >> 8);
}

void RTPMedia::OnRTCPCompoundPacket(RTCPCompoundPacket * pack, const RTPTime & receivetime, const RTPAddress * senderaddress)
{
}

void RTPMedia::OnPollThreadStep()
{
}

void RTPMedia::OnPollThreadError(int status)
{
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
		if (SendPacket((void *)(&event_hdr), sizeof(event_hdr), (unsigned char)TELEPHONE_EVENT, true, (unsigned long)0) < 0)
			break;

		event_hdr.duration += duration;
		if (SendPacket((void *)(&event_hdr), sizeof(event_hdr), (unsigned char)TELEPHONE_EVENT, false, (unsigned long)0) < 0)
			break;

		event_hdr.duration += duration;
		event_hdr.E = 1;
		if (SendPacket((void *)(&event_hdr), sizeof(event_hdr), (unsigned char)TELEPHONE_EVENT, false, (unsigned long)event_hdr.duration) < 0)
			break;

		ret = duration;
	} while (0);
	
	return ret;
}
