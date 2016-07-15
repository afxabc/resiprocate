
#include "RccUserAgent.h"



void RccMessage::netToHost() const
{
	mSize = ntohs(mSize);
	switch (mType)
	{
	case CALL_INVITE:
		rccInvite.mRtpIP = ntohl(rccInvite.mRtpIP);
		rccInvite.mRtpPort = ntohs(rccInvite.mRtpPort);
		rccInvite.mRtpRate = ntohl(rccInvite.mRtpRate);
		break;
	case CALL_ACCEPT:
		rccAccept.mRtpIP = ntohl(rccAccept.mRtpIP);
		rccAccept.mRtpPort = ntohs(rccAccept.mRtpPort);
		rccAccept.mRtpRate = ntohl(rccAccept.mRtpRate);
		break;
	}

}

void RccMessage::hostToNet() const
{
	mSize = htons(mSize);
	switch (mType)
	{
	case CALL_INVITE:
		rccInvite.mRtpIP = htonl(rccInvite.mRtpIP);
		rccInvite.mRtpPort = htons(rccInvite.mRtpPort);
		rccInvite.mRtpRate = htonl(rccInvite.mRtpRate);
		break;
	case CALL_ACCEPT:
		rccAccept.mRtpIP = htonl(rccAccept.mRtpIP);
		rccAccept.mRtpPort = htons(rccAccept.mRtpPort);
		rccAccept.mRtpRate = htonl(rccAccept.mRtpRate);
		break;
	}
}

////////////////////////////////////////////////////////////////////

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

	int tryCount = 10;
	while ((mSocket = openPort(mLocalPort, mLocalIP, false)) == INVALID_SOCKET && tryCount > 0)
	{
		mLocalPort++;
		mTargetPort++;
		tryCount--;
	}
		
	if (mSocket == INVALID_SOCKET)
		return false;

	std::cout << "RccUserAgent start at port " << mLocalPort << std::endl;

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
	msg.hostToNet();
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

bool RccUserAgent::sendMessageResult(bool ok, RccMessage::MessageType which)
{
	RccMessage msg;
	msg.mType = RccMessage::CALL_RESULT;
	msg.rccResult.which = which;
	msg.rccResult.ok = ok;
	msg.mSize = RccMessage::HEAD_SIZE + sizeof(RccMessage::rccResult);
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
	msg->mSize = RccMessage::HEAD_SIZE + sizeof(RccMessage::rccRegister)+strlen(callNumber);

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
	msg->rccInvite.mRtpIP = ntohl(inet_addr(rtpIP));
	msg->rccInvite.mRtpPort = rtpPort;
	msg->rccInvite.mRtpPayload = payload;
	msg->rccInvite.mRtpRate = rate;
	strcpy(msg->rccInvite.mCallNum, callNumber);
	msg->mSize = RccMessage::HEAD_SIZE + sizeof(RccMessage::rccInvite) + strlen(callNumber);

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
	msg->rccAccept.mRtpIP = ntohl(inet_addr(rtpIP));
	msg->rccAccept.mRtpPort = rtpPort;
	msg->rccAccept.mRtpPayload = payload;
	msg->rccAccept.mRtpRate = rate;
	msg->mSize = sizeof(RccMessage);
	msg->mSize = RccMessage::HEAD_SIZE + sizeof(RccMessage::rccAccept);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessageClose(unsigned char reason)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->mType = RccMessage::CALL_CLOSE;
	msg->rccClose.mReason = reason;
	msg->mSize = RccMessage::HEAD_SIZE + sizeof(RccMessage::rccClose);

	return sendMessage(*msg);
}

int RccUserAgent::getMessage(RccMessage * msg, int sz)
{
	if (mSocket == INVALID_SOCKET || msg == NULL)
		return 0;

	if (!::getMessage(mSocket, (char*)msg, &sz, &mTargetIP, &mTargetPort, false))
		return 0;

	msg->netToHost();

	return sz;
}
