#if !defined(RccUserAgent_hxx)
#define RccUserAgent_hxx

#include "rutil/stun/Udp.hxx"
#include "rutil/Socket.hxx"


//测试使用的RCC服务端口
static const unsigned short DUMMY_RCC_PORT = 9767;
///////////////////////


#ifdef WIN32
#pragma pack(1)
#endif

struct RccMessage
{
	enum MessageType
	{
		CALL_REGISTER,
		CALL_INVITE,
		CALL_CLOSE,
		CALL_ACCEPT,
		CALL_ANSWER,
		CALL_RING,
		CALL_RINGBACK,
		CALL_CONNECTED,
		CALL_FAILED
	};

	RccMessage() : mSize(sizeof(RccMessage)) {}

	MessageType mType;
	unsigned short mSize;
	unsigned int mRtpIP;
	unsigned short mRtpPort;
	unsigned char mRtpPayload;
	char mCallNum[1];
}
#ifndef WIN32
__attribute__((packed))
#endif
;

#ifdef WIN32
#pragma pack() 
#endif

class RccUserAgent
{
public:
	RccUserAgent();
	~RccUserAgent();

	bool startAgent(unsigned short localPort, const char* localIP = NULL, unsigned short targetPort = 0, const char* targetIP = NULL);
	void stopAgent();

	bool isValid() { return (mSocket != INVALID_SOCKET); }

	bool sendMessage(const RccMessage& msg);
	bool sendMessage(RccMessage::MessageType type);
	bool sendMessage(RccMessage::MessageType type, const char * callNumber);
	bool sendMessage(RccMessage::MessageType type, const char * callNumber, const char* rtpIP, unsigned short rtpPort, int payload);
	int getMessage(RccMessage* msg, int sz = sizeof(RccMessage));

private:
	resip::Socket mSocket;
	UInt32 mLocalIP;
	unsigned short mLocalPort;
	UInt32 mTargetIP;
	unsigned short mTargetPort;
};

#endif