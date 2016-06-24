
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

bool RccUserAgent::sendMessage(RccMessage::MessageType type)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	RccMessage msg;
	msg.mType = type;
	return sendMessage(msg);
}

bool RccUserAgent::sendMessage(const RccMessage& msg)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	int slen = (msg.mSize > 0) ? msg.mSize : sizeof(msg);
	return ::sendMessage(mSocket, (char*)&msg, slen, mTargetIP, mTargetPort, false);
}

bool RccUserAgent::sendMessage(RccMessage::MessageType type, const char * callNumber)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[1024];
	memset(data, 1024, 0);

	RccMessage* msg = (RccMessage*)data;
	msg->mType = type;
	strcpy(msg->mCallNum, callNumber);
	msg->mSize = sizeof(RccMessage)+strlen(callNumber);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessage(RccMessage::MessageType type, const char * callNumber, const char * rtpIP, unsigned short rtpPort, int payload)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[1024];
	memset(data, 1024, 0);

	RccMessage* msg = (RccMessage*)data;
	msg->mType = type;
	msg->mRtpIP = inet_addr(rtpIP);// ntohl(inet_addr(rtpIP));
	msg->mRtpPort = rtpPort;
	msg->mRtpPayload = payload;
	strcpy(msg->mCallNum, callNumber);
	msg->mSize = sizeof(RccMessage)+strlen(callNumber);

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
