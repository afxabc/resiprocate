#if !defined(RccUserAgent_hxx)
#define RccUserAgent_hxx

#include "rutil/stun/Udp.hxx"
#include "rutil/Socket.hxx"


//测试使用的RCC服务端口
static const unsigned short RCC_SERVER_PORT_BASE = 22000;
static const unsigned short RCC_CLIENT_PORT_BASE = 24000;
static const unsigned short RCC_RTP_PORT_BASE = 28000;
///////////////////////


#ifdef WIN32
#pragma pack(1)
#endif

class RccUserAgent;
struct RccMessage
{
	static const int ALLOC_SIZE = 1024;
	enum MessageType
	{
		CALL_REGISTER,
		CALL_UNREGISTER,
		CALL_INVITE,
		CALL_CLOSE,
		CALL_ACCEPT,
		CALL_CONNECTED,
		CALL_TRYING,
		CALL_RESULT,
	};

	RccMessage() : mSize(HEAD_SIZE) {}

private:
	void netToHost() const;
	void hostToNet() const;

	friend class RccUserAgent;

public:
	//////////////////////////
	//以下均为网络字序
	/////////////////////////

	static const int HEAD_SIZE = 3;	//type+size
	unsigned char mType;
	mutable short mSize;

	//以下消息需要附加数据
	union 
	{
		struct RccRegister
		{
			char mCallNum[1];			
		}rccRegister;

		struct RccInvite
		{
			mutable unsigned int mRtpIP;			//rtp地址
			mutable unsigned short mRtpPort;		//rtp端口
			mutable unsigned char mRtpPayload;		//rtp编码类型
			mutable unsigned int mRtpRate;			//rtp码率
			char mCallNum[1];
		}rccInvite;

		struct RccAccept
		{
			mutable unsigned int mRtpIP;
			mutable unsigned short mRtpPort;
			mutable unsigned char mRtpPayload;
			mutable unsigned int mRtpRate;
		}rccAccept;

		/*
		0:  "ended due to an error"
		1:  "ended due to a timeout"
		2:  "ended due to being replaced"
		3:  "ended locally via BYE"
		4:  "received a BYE from peer"
		5:  "ended locally via CANCEL"
		6:  "received a CANCEL from peer";
		7:  "received a rejection from peer"
		*/
		struct RccClose
		{
			unsigned char mReason;
		}rccClose;

		struct RccResult
		{
			unsigned char which;		//MessageType
			bool ok;
		}rccResult;
	};
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

	bool sendMessage(RccMessage::MessageType type);
	bool sendMessageResult(bool ok, RccMessage::MessageType which);
	bool sendMessageRegister(const char * callNumber);
	bool sendMessageInvite(const char * callNumber, const char* rtpIP, unsigned short rtpPort, unsigned char payload, unsigned int rate);
	bool sendMessageAccept(const char* rtpIP, unsigned short rtpPort, unsigned char payload, unsigned int rate);
	bool sendMessageClose(unsigned char reason = 0);
	int getMessage(RccMessage* msg, int sz = sizeof(RccMessage));

	unsigned short localPort() { return mLocalPort; }

private:
	bool sendMessage(const RccMessage& msg);

private:
	resip::Socket mSocket;
	UInt32 mLocalIP;
	unsigned short mLocalPort;
	UInt32 mTargetIP;
	unsigned short mTargetPort;
};

#endif