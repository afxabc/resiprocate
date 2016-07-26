#if !defined(RccUserAgent_hxx)
#define RccUserAgent_hxx

#include "rutil/stun/Udp.hxx"
#include "rutil/Socket.hxx"


//����ʹ�õ�RCC����˿�
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
	static const int HEAD_SIZE = 5;	//type+size+version
	static const int CALLNUM_ALLOC_SIZE = 16;

	struct RtpDataI
	{
		mutable unsigned int ip;			//rtp��ַ
		mutable unsigned short port;		//rtp�˿�
		mutable unsigned char payload;		//rtp��������
		mutable unsigned int rate;			//rtp����
	};

	enum MessageType
	{
		RCC_RGST = 1,				//ע��
		RCC_URGST,					//ע��
		RCC_IAM,					//�������
		RCC_ACM,					//��Ϣ��Ӧ
		RCC_ANM,					//Ӧ��
		RCC_REL,					//�ͷ�
		RCC_CONN,					//����
		RCC_OPTION,					//��չ
		RCC_TXT,					//��Ϣ
		RCC_END
	};

	RccMessage() : size(HEAD_SIZE), version(0) {}

private:
	void netToHost() const;
	void hostToNet() const;

	friend class RccUserAgent;

public:
	//////////////////////////
	//���¾�Ϊ��������
	/////////////////////////

	mutable unsigned short version;	//
	mutable unsigned char type;
	mutable unsigned short size;

	//������Ϣ��Ҫ��������
	union 
	{
		struct RccRgst
		{
			unsigned char callNumLength;
			char callNum[CALLNUM_ALLOC_SIZE];
		}rccRgst;

		struct RccUrgst
		{
			unsigned char callNumLength;
			char callNum[CALLNUM_ALLOC_SIZE];
		}rccUrgst;

		struct RccIam
		{
			unsigned char userType;		//0Ah ��ͨ�����û�
			unsigned char callNumLength;
			char callNum[CALLNUM_ALLOC_SIZE];
			unsigned char rtpCount;
			mutable RtpDataI rtpData[1];
		}rccIam;

		struct RccAnm
		{
			unsigned char rtpCount;
			mutable RtpDataI rtpData[1];
		}rccAnm;

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
		struct RccRel
		{
			unsigned char reason;
		}rccRel;

		struct RccAcm
		{
			unsigned char which;		//MessageType
			unsigned char result;		//0�����������쳣
		}rccAcm;

		struct RccTxt
		{
			unsigned char callNumLength;
			char callNum[CALLNUM_ALLOC_SIZE];
			mutable unsigned short txtLength;
			char txtData[1];
		}rccTxt;

	};
}
#ifndef WIN32
__attribute__((packed))
#endif
;

#ifdef WIN32
#pragma pack() 
#endif

class RccRtpData
{
public:
	RccRtpData() {}
	RccRtpData(const char* _ip, unsigned short _port, unsigned char _payload, unsigned int _rate)
		: ip(_ip), port(_port), payload(_payload), rate(_rate) {}

	std::string ip;				//rtp��ַ
	unsigned short port;		//rtp�˿�
	unsigned char payload;		//rtp��������
	unsigned int rate;			//rtp����
};

typedef std::vector<RccRtpData> RccRtpDataList;

class IRccMessageCallback
{
public:
	virtual void onMessage(RccMessage::MessageType type) = 0;
	virtual void onMessageAcm(RccMessage::MessageType which, unsigned char result) = 0;
	virtual void onMessageRgst(const char * callNumber) = 0;
	virtual void onMessageUrgst(const char * callNumber) = 0;
	virtual void onMessageRel(unsigned char reason) = 0;
	virtual void onMessageIam(const char * callNumber, RccRtpDataList& rtpDataList) = 0;
	virtual void onMessageAnm(RccRtpDataList& rtpDataList) = 0;
	virtual void onMessageTxt(const char * callNumber, const char * txt, unsigned short len) = 0;

	virtual void onInvalidMessage(RccMessage* msg) = 0;
};

class RccUserAgent
{
public:
	RccUserAgent();
	~RccUserAgent();

	bool startAgent(unsigned short localPort, const char* localIP = NULL, unsigned short targetPort = 0, const char* targetIP = NULL);
	void stopAgent();

	bool isValid() { return (mSocket != INVALID_SOCKET); }

	bool sendMessage(RccMessage::MessageType type);
	bool sendMessageAcm(RccMessage::MessageType which, unsigned char result = 0);
	bool sendMessageRgst(const char * callNumber);
	bool sendMessageUrgst(const char * callNumber);
	bool sendMessageRel(unsigned char reason = 0);
	bool sendMessageIam(const char * callNumber, const RccRtpData& rtpData);
	bool sendMessageIam(const char * callNumber, const RccRtpDataList& rtpDataList);
	bool sendMessageAnm(const RccRtpData& rtpData);
	bool sendMessageAnm(const RccRtpDataList& rtpDataList);
	bool sendMessageTxt(const char * callNumber, const char * txt, unsigned short len);

	int getMessage(char* buff, int sz);
	void dispatchMessage(const char* buff, IRccMessageCallback* cb);
	void getAndDispatchMessage(IRccMessageCallback* cb);

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