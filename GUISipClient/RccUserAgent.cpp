
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

bool RccUserAgent::startAgent(UInt32 localIP, unsigned short localPort, UInt32 targetIP, unsigned short targetPort)
{
	stopAgent();
	
	mSocket = openPort(localPort, localIP, false);
	if (mSocket == INVALID_SOCKET)
		return false;

	mLocalIP = localIP;
	mLocalPort = localPort;
	mTargetIP = targetIP;
	mTargetPort = targetPort;

	resip::makeSocketNonBlocking(mSocket);

	return true;
}

void RccUserAgent::stopAgent()
{
	if (mSocket == INVALID_SOCKET)
		return;

	resip::closeSocket(mSocket);
	mSocket = INVALID_SOCKET;

	mLocalIP = 0;
	mLocalPort = 0;
	mTargetIP = 0;
	mTargetPort = 0;
}

bool RccUserAgent::sendMessage(RccMessage::MessageType type)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	RccMessage msg;
	msg.mType = type;
	msg.mData[0] = 0;
	return ::sendMessage(mSocket, (char*)&msg, sizeof(msg), mTargetIP, mTargetPort, false);
}

bool RccUserAgent::sendMessage(RccMessage::MessageType type, const char * callNumber)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[1024];
	memset(data, 1024, 0);
	RccMessage* msg = (RccMessage*)data;
	msg->mType = type;
	strcpy(msg->mData, callNumber);

	return ::sendMessage(mSocket, data, sizeof(msg)+strlen(callNumber), mTargetIP, mTargetPort, false);
}

int RccUserAgent::getMessage(RccMessage * msg, int sz)
{
	if (mSocket == INVALID_SOCKET || msg == NULL)
		return 0;

	if (!::getMessage(mSocket, (char*)msg, &sz, &mTargetIP, &mTargetPort, false))
		return 0;

	return sz;
}
