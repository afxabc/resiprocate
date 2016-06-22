#if !defined(RccUserAgent_hxx)
#define RccUserAgent_hxx

#include "rutil/stun/Udp.hxx"
#include "rutil/Socket.hxx"

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
		CALL_RING,
		CALL_RINGBACK,
		CALL_FAILED
	};

	MessageType mType;
	char mData[1];
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

	bool startAgent(UInt32 localIP, unsigned short localPort, UInt32 targetIP = 0, unsigned short targetPort = 0);
	void stopAgent();

	bool isValid() { return (mSocket != INVALID_SOCKET); }

	bool sendMessage(RccMessage::MessageType type);
	bool sendMessage(RccMessage::MessageType type, const char* callNumber);
	int getMessage(RccMessage* msg, int sz = sizeof(RccMessage));

private:
	resip::Socket mSocket;
	UInt32 mLocalIP;
	unsigned short mLocalPort;
	UInt32 mTargetIP;
	unsigned short mTargetPort;
};

#endif