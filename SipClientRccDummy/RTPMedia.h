#ifndef RTPMEDIA_HEADDER_H_H__
#define RTPMEDIA_HEADDER_H_H__

#include "jrtplib3\rtpsession.h"
#include "jrtplib3\rtppacket.h"
#include "SipClientUdp\RccUserAgent.h"
#include "rutil\ThreadIf.hxx"

class RTPMedia;
class IRTPMediaCallback
{
public:
	virtual void onMediaData(char* data, int len, unsigned char payload, unsigned short seq) = 0;
};

class RTPMedia : public resip::ThreadIf
{
public:
	RTPMedia(IRTPMediaCallback* cb, const char* ip, unsigned short port, unsigned char payload, unsigned int rate);
	~RTPMedia();

	void setRemote(const std::string& ip, unsigned short port, unsigned char payload, unsigned int rate);
	void setRemoteSelf();
	bool start(unsigned int ptime);
	unsigned short tryPort(unsigned short port);
	void stop();
	int sendData(char* data, int len);
	int sendData(char *data, int len, unsigned char pt, bool mark, unsigned long timestampinc);
	int sendDtmfKey(int evt, int vol, int duration);

	std::string& ip() { return rtpIP_; }
	unsigned short& port() { return rtpPort_; }
	unsigned char& payload() { return rtpPayload_; }
	unsigned int& rate() { return rtpRate_; }

	std::string& r_ip() { return remoteRtpIP_; }
	unsigned short& r_port() { return remoteRtpPort_; }
	unsigned char& r_payload() { return remoteRtpPayload_; }
	unsigned int& r_rate() { return remoteRtpRate_; }

	static const int MAX_PACKET_SIZE = 1280;

private:
	IRTPMediaCallback* cb_;

	resip::Mutex mutex_;
	jrtplib::RTPSession rtpSession_;
	unsigned short hdrextID_;

	std::string rtpIP_;
	unsigned short rtpPort_;
	unsigned char rtpPayload_;
	unsigned int rtpRate_;

	std::string remoteRtpIP_;
	unsigned short remoteRtpPort_;
	unsigned char remoteRtpPayload_;
	unsigned int remoteRtpRate_;


	// Í¨¹ý ThreadIf ¼Ì³Ð
	virtual void thread() override;

};

#endif
