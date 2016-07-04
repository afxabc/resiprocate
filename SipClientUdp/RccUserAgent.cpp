
#include "RccUserAgent.h"



void RccMessage::netToHost() const
{
	char* strIP = NULL;
	mSize = ntohs(mSize);
	switch (mType)
	{
	case CALL_INVITE:
		rccInvite.mRtpPort = ntohs(rccInvite.mRtpPort);
		rccInvite.mRtpRate = ntohl(rccInvite.mRtpRate);
		strIP = rccInvite.mRtpIP;
		break;
	case CALL_ACCEPT:
		rccAccept.mRtpPort = ntohs(rccAccept.mRtpPort);
		rccAccept.mRtpRate = ntohl(rccAccept.mRtpRate);
		strIP = rccAccept.mRtpIP;
		break;
	}

	if (strIP == NULL)
		return;

	//make sure mRtpIP ended with 0
	int i;
	for (i = 0; i < RccMessage::IP_STR_SIZE; ++i)
	{
		if (strIP[i] == 0)
			break;

		if (strIP[i] != '.' && !isdigit(strIP[i]))
		{
			strIP[i] = 0;
			break;
		}
	}
	if (i == RccMessage::IP_STR_SIZE)
		strIP[i] = 0;
}

void RccMessage::hostToNet() const
{
	mSize = htons(mSize);
	switch (mType)
	{
	case CALL_INVITE:
		rccInvite.mRtpPort = htons(rccInvite.mRtpPort);
		rccInvite.mRtpRate = htonl(rccInvite.mRtpRate);
		break;
	case CALL_ACCEPT:
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
	//msg->rccInvite.mRtpIP = inet_addr(rtpIP);// ntohl(inet_addr(rtpIP));
	strncpy(msg->rccInvite.mRtpIP, rtpIP, RccMessage::IP_STR_SIZE);
	msg->rccInvite.mRtpPort = rtpPort;
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
	//msg->rccAccept.mRtpIP = inet_addr(rtpIP);// ntohl(inet_addr(rtpIP));
	strncpy(msg->rccInvite.mRtpIP, rtpIP, RccMessage::IP_STR_SIZE);
	msg->rccAccept.mRtpPort = rtpPort;
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

	msg->netToHost();

	return sz;
}
