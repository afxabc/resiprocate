
#include "RccUserAgent.h"


RccUserAgent::RccUserAgent() :
	mLocalIP(0),
	mLocalPort(0),
	mTargetIP(0),
	mTargetPort(0),
	mSocket(INVALID_SOCKET)
{
}

RccUserAgent::~RccUserAgent()
{
	stopAgent();
}

bool RccUserAgent::startAgent(unsigned short localPort, const char* localIP, unsigned short targetPort, const char* targetIP)
{
	stopAgent();
	
	if (localIP != NULL)
		mLocalIP = ntohl(inet_addr(localIP));
	mLocalPort = localPort;

	if (targetIP != NULL)
		mTargetIP = ntohl(inet_addr(targetIP));
	mTargetPort = targetPort;

	mSocket = openPort(mLocalPort, mLocalIP, false);
	if (mSocket == INVALID_SOCKET)
		return false;

	resip::makeSocketNonBlocking(mSocket);

	return true;
}

void RccUserAgent::stopAgent()
{
	mLocalIP = 0;
	mLocalPort = 0;
	mTargetIP = 0;
	mTargetPort = 0;

	if (mSocket == INVALID_SOCKET)
		return;

	resip::closeSocket(mSocket);
	mSocket = INVALID_SOCKET;

}

bool RccUserAgent::sendMessage(const RccMessage& msg)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	int slen = (msg.mSize > 0) ? msg.mSize : sizeof(msg);
	return ::sendMessage(mSocket, (char*)&msg, slen, mTargetIP, mTargetPort, false);
}

bool RccUserAgent::sendMessage(RccMessage::MessageType type)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	RccMessage msg;
	msg.mType = type;
	return sendMessage(msg);
}

bool RccUserAgent::sendMessageRegister(const char * callNumber)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->mType = RccMessage::CALL_REGISTER;
	strcpy(msg->rccRegister.mCallNum, callNumber);
	msg->mSize = sizeof(RccMessage)+strlen(callNumber);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessageInvite(const char * callNumber, const char * rtpIP, unsigned short rtpPort, unsigned char payload, unsigned int rate)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->mType = RccMessage::CALL_INVITE;
	msg->rccInvite.mRtpIP = inet_addr(rtpIP);// ntohl(inet_addr(rtpIP));
	msg->rccInvite.mRtpPort = htons(rtpPort);
	msg->rccInvite.mRtpPayload = payload;
	msg->rccInvite.mRtpRate = rate;
	strcpy(msg->rccInvite.mCallNum, callNumber);
	msg->mSize = sizeof(RccMessage)+strlen(callNumber);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessageAccept(const char * rtpIP, unsigned short rtpPort, unsigned char payload, unsigned int rate)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->mType = RccMessage::CALL_ACCEPT;
	msg->rccAccept.mRtpIP = inet_addr(rtpIP);// ntohl(inet_addr(rtpIP));
	msg->rccAccept.mRtpPort = htons(rtpPort);
	msg->rccAccept.mRtpPayload = payload;
	msg->rccAccept.mRtpRate = rate;
	msg->mSize = sizeof(RccMessage);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessageClose(unsigned char error, const char * reason)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->mType = RccMessage::CALL_CLOSE;
	msg->rccClose.mError = error;
	if (reason != NULL)
	{
		strcpy(msg->rccClose.mReason, reason);
		msg->mSize = sizeof(RccMessage) + strlen(reason);
	}
	else msg->mSize = sizeof(RccMessage);
	
	return sendMessage(*msg);
}

int RccUserAgent::getMessage(RccMessage * msg, int sz)
{
	if (mSocket == INVALID_SOCKET || msg == NULL)
		return 0;

	if (!::getMessage(mSocket, (char*)msg, &sz, &mTargetIP, &mTargetPort, false))
		return 0;

	return sz;
}
