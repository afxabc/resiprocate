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

	RccMessage() : mSize(sizeof(RccMessage)) {}

private:
	void netToHost() const;
	void hostToNet() const;

	friend class RccUserAgent;

public:
	//////////////////////////
	//以下short，int为网络字序
	/////////////////////////

	unsigned char mType;
	mutable short mSize;
		
	static const int IP_STR_SIZE = 18;

	//以下消息需要附加数据
	union 
	{
		struct RccRegister
		{
			char mCallNum[1];			
		}rccRegister;

		struct RccInvite
		{
			mutable char mRtpIP[IP_STR_SIZE+1];		//rtp地址
			mutable unsigned short mRtpPort;		//rtp端口
			unsigned char mRtpPayload;				//rtp编码类型
			mutable unsigned int mRtpRate;			//rtp码率
			char mCallNum[1];
		}rccInvite;

		struct RccAccept
		{
			mutable char mRtpIP[IP_STR_SIZE+1];
			mutable unsigned short mRtpPort;
			unsigned char mRtpPayload;
			mutable unsigned int mRtpRate;
		}rccAccept;

		struct RccClose
		{
			unsigned char mError;
			char mReason[1];
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
	bool sendMessageClose(unsigned char error = 0, const char* reason = NULL);
	int getMessage(RccMessage* msg, int sz = sizeof(RccMessage));

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