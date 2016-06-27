#if !defined(RccUserAgent_hxx)
#define RccUserAgent_hxx

#include "rutil/stun/Udp.hxx"
#include "rutil/Socket.hxx"


//����ʹ�õ�RCC����˿�
static const unsigned short DUMMY_RCC_PORT = 9767;
///////////////////////


#ifdef WIN32
#pragma pack(1)
#endif

struct RccMessage
{
	static const int ALLOC_SIZE = 1024;
	enum MessageType
	{
		CALL_REGISTER,
		CALL_INVITE,
		CALL_CLOSE,
		CALL_ACCEPT,
		CALL_CONNECTED
	};

	RccMessage() : mSize(sizeof(RccMessage)) {}

	MessageType mType;
	unsigned short mSize;
	//������Ϣ��Ҫ��������
	union 
	{
		struct RccRegister
		{
			char mCallNum[1];			
		}rccRegister;

		struct RccInvite
		{
			//����������ͬ
			unsigned int mRtpIP;			//rtp��ַ
			unsigned short mRtpPort;		//rtp�˿�
			unsigned char mRtpPayload;		//rtp��������
			unsigned int mRtpRate;			//rtp����
			char mCallNum[1];
		}rccInvite;

		struct RccAccept
		{
			unsigned int mRtpIP;
			unsigned short mRtpPort;
			unsigned char mRtpPayload;
			unsigned int mRtpRate;			
		}rccAccept;

		struct RccClose
		{
			unsigned char mError;
			char mReason[1];
		}rccClose;
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

	bool sendMessage(const RccMessage& msg);
	bool sendMessage(RccMessage::MessageType type);
	bool sendMessageRegister(const char * callNumber);
	bool sendMessageInvite(const char * callNumber, const char* rtpIP, unsigned short rtpPort, unsigned char payload, unsigned int rate);
	bool sendMessageAccept(const char* rtpIP, unsigned short rtpPort, unsigned char payload, unsigned int rate);
	bool sendMessageClose(unsigned char error = 0, const char* reason = NULL);
	int getMessage(RccMessage* msg, int sz = sizeof(RccMessage));

private:
	resip::Socket mSocket;
	UInt32 mLocalIP;
	unsigned short mLocalPort;
	UInt32 mTargetIP;
	unsigned short mTargetPort;
};

#endif