
#include "RccUserAgent.h"


void RccMessage::netToHost() const
{
	size = ntohs(size);

	int rtpCount = 0;
	RtpDataI* pRtpData = NULL;

	switch (type)
	{
	case RCC_IAM:
		rtpCount = rccIam.rtpCount;
		pRtpData = rccIam.rtpData;
		break;
	case RCC_ANM:
		rtpCount = rccAnm.rtpCount;
		pRtpData = rccAnm.rtpData;
		break;
	}

	for (int i = 0; i < rtpCount; i++)
	{
		pRtpData[i].ip = ntohl(pRtpData[i].ip);
		pRtpData[i].port = ntohs(pRtpData[i].port);
		pRtpData[i].rate = ntohl(pRtpData[i].rate);
	}
}

void RccMessage::hostToNet() const
{
	size = htons(size);

	int rtpCount = 0;
	RtpDataI* pRtpData = NULL;

	switch (type)
	{
	case RCC_IAM:
		rtpCount = rccIam.rtpCount;
		pRtpData = rccIam.rtpData;
		break;
	case RCC_ANM:
		rtpCount = rccAnm.rtpCount;
		pRtpData = rccAnm.rtpData;
		break;
	}

	for (int i = 0; i < rtpCount; i++)
	{
		pRtpData[i].ip = htonl(pRtpData[i].ip);
		pRtpData[i].port = htons(pRtpData[i].port);
		pRtpData[i].rate = htonl(pRtpData[i].rate);
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

	int slen = (msg.size > 0) ? msg.size+RccMessage::HEAD_SIZE : RccMessage::HEAD_SIZE;
	msg.hostToNet();
	return ::sendMessage(mSocket, (char*)&msg, slen, mTargetIP, mTargetPort, false);
}

bool RccUserAgent::sendMessage(RccMessage::MessageType type)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	RccMessage msg;
	msg.type = type;
	return sendMessage(msg);
}

bool RccUserAgent::sendMessageAcm(RccMessage::MessageType which, unsigned char result)
{
	RccMessage msg;
	msg.type = RccMessage::RCC_ACM;
	msg.rccAcm.which = which;
	msg.rccAcm.result = result;
	msg.size = sizeof(RccMessage::rccAcm);
	return sendMessage(msg);
}

bool RccUserAgent::sendMessageRgst(const char * callNumber)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->type = RccMessage::RCC_RGST;
	msg->rccRgst.callNumLength = (unsigned char)strlen(callNumber);
	strncpy(msg->rccRgst.callNum, callNumber, RccMessage::CALLNUM_ALLOC_SIZE);
	msg->size = sizeof(RccMessage::rccRgst);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessageUrgst(const char * callNumber)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->type = RccMessage::RCC_URGST;
	msg->rccUrgst.callNumLength = (unsigned char)strlen(callNumber);
	strncpy(msg->rccUrgst.callNum, callNumber, RccMessage::CALLNUM_ALLOC_SIZE);
	msg->size = sizeof(RccMessage::rccUrgst);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessageIam(const char * callNumber, const RccRtpData& rtpData)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->type = RccMessage::RCC_IAM;

	msg->rccIam.userType = 0x0a;
	msg->rccIam.callNumLength = (unsigned char)strlen(callNumber);
	strncpy(msg->rccIam.callNum, callNumber, RccMessage::CALLNUM_ALLOC_SIZE);

	msg->rccIam.rtpCount = 1;
	msg->rccIam.rtpData[0].ip = ntohl(inet_addr(rtpData.ip.c_str()));
	msg->rccIam.rtpData[0].port = rtpData.port;
	msg->rccIam.rtpData[0].payload = rtpData.payload;
	msg->rccIam.rtpData[0].rate = rtpData.rate;

	msg->size = sizeof(RccMessage::rccIam);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessageIam(const char * callNumber, const RccRtpDataList& rtpDataList)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->type = RccMessage::RCC_IAM;

	msg->rccIam.userType = 0x0a;
	msg->rccIam.callNumLength = (unsigned char)strlen(callNumber);
	strncpy(msg->rccIam.callNum, callNumber, RccMessage::CALLNUM_ALLOC_SIZE);

	msg->rccIam.rtpCount = (unsigned char)rtpDataList.size();
	for (unsigned int i = 0; i < rtpDataList.size(); ++i)
	{
		msg->rccIam.rtpData[i].ip = ntohl(inet_addr(rtpDataList[i].ip.c_str()));
		msg->rccIam.rtpData[i].port = rtpDataList[i].port;
		msg->rccIam.rtpData[i].payload = rtpDataList[i].payload;
		msg->rccIam.rtpData[i].rate = rtpDataList[i].rate;
	}
	
	msg->size = sizeof(RccMessage::rccIam)+sizeof(RccMessage::RtpDataI)*(msg->rccIam.rtpCount-1);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessageAnm(const RccRtpData& rtpData1)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->type = RccMessage::RCC_ANM;

	msg->rccAnm.rtpCount = 1;
	msg->rccAnm.rtpData[0].ip = ntohl(inet_addr(rtpData1.ip.c_str()));
	msg->rccAnm.rtpData[0].port = rtpData1.port;
	msg->rccAnm.rtpData[0].payload = rtpData1.payload;
	msg->rccAnm.rtpData[0].rate = rtpData1.rate;
	
	msg->size = sizeof(RccMessage::rccAnm);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessageAnm(const RccRtpDataList& rtpDataList)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	char data[RccMessage::ALLOC_SIZE];
	memset(data, 0, RccMessage::ALLOC_SIZE);

	RccMessage* msg = (RccMessage*)data;
	msg->type = RccMessage::RCC_ANM;

	msg->rccAnm.rtpCount = (unsigned char)rtpDataList.size();
	for (unsigned int i = 0; i < rtpDataList.size(); ++i)
	{
		msg->rccAnm.rtpData[i].ip = ntohl(inet_addr(rtpDataList[i].ip.c_str()));
		msg->rccAnm.rtpData[i].port = rtpDataList[i].port;
		msg->rccAnm.rtpData[i].payload = rtpDataList[i].payload;
		msg->rccAnm.rtpData[i].rate = rtpDataList[i].rate;
	}

	msg->size = sizeof(RccMessage::rccAnm) + sizeof(RccMessage::RtpDataI)*(msg->rccAnm.rtpCount - 1);

	return sendMessage(*msg);
}

bool RccUserAgent::sendMessageRel(unsigned char reason)
{
	if (mSocket == INVALID_SOCKET)
		return false;

	RccMessage msg;
	msg.type = RccMessage::RCC_REL;
	msg.rccRel.reason = reason;
	msg.size = sizeof(RccMessage::rccRel);
	return sendMessage(msg);
}

int RccUserAgent::getMessage(char * data, int sz)
{
	if (mSocket == INVALID_SOCKET)
		return 0;

	if (!::getMessage(mSocket, data, &sz, &mTargetIP, &mTargetPort, false))
		return 0;

	return sz;
}

void RccUserAgent::dispatchMessage(const char * data, IRccMessageCallback * cb)
{
	if (data == NULL || cb == NULL)
		return;

	RccMessage* msg = (RccMessage*)data;
	msg->netToHost();

	switch (msg->type)
	{
	case RccMessage::RCC_ACM:
		cb->onMessageAcm((RccMessage::MessageType)msg->rccAcm.which, msg->rccAcm.result);
		break;
	case RccMessage::RCC_RGST:
	{
		if (msg->rccRgst.callNumLength <= RccMessage::CALLNUM_ALLOC_SIZE && msg->rccRgst.callNumLength > 0)
		{
			char callNum[RccMessage::CALLNUM_ALLOC_SIZE + 1];
			memset(callNum, 0, RccMessage::CALLNUM_ALLOC_SIZE + 1);
			memcpy(callNum, msg->rccRgst.callNum, msg->rccRgst.callNumLength);
			cb->onMessageRgst(callNum);
		}
		else cb->onInvalidMessage(msg);
		break;
	}
	case RccMessage::RCC_URGST:
	{
		if (msg->rccUrgst.callNumLength <= RccMessage::CALLNUM_ALLOC_SIZE && msg->rccUrgst.callNumLength > 0)
		{
			char callNum[RccMessage::CALLNUM_ALLOC_SIZE + 1];
			memset(callNum, 0, RccMessage::CALLNUM_ALLOC_SIZE + 1);
			memcpy(callNum, msg->rccUrgst.callNum, msg->rccUrgst.callNumLength);
			cb->onMessageUrgst(callNum);
		}
		else cb->onInvalidMessage(msg);
		break;
	}
	case RccMessage::RCC_REL:
		cb->onMessageRel(msg->rccRel.reason);
		break;
	case RccMessage::RCC_IAM:
	{
		if (msg->rccIam.callNumLength > RccMessage::CALLNUM_ALLOC_SIZE || msg->rccIam.callNumLength == 0
			|| msg->rccIam.rtpCount > 8 || msg->rccIam.rtpCount == 0)
		{
			cb->onInvalidMessage(msg);
			break;
		}

		char callNum[RccMessage::CALLNUM_ALLOC_SIZE + 1];
		memset(callNum, 0, RccMessage::CALLNUM_ALLOC_SIZE + 1);
		memcpy(callNum, msg->rccIam.callNum, msg->rccIam.callNumLength);

		RccRtpDataList rtpDataList;
		for (int i = 0; i<msg->rccIam.rtpCount; ++i)
		{
			struct in_addr addr;
			addr.S_un.S_addr = htonl(msg->rccIam.rtpData[i].ip);
			RccRtpData rtpData(inet_ntoa(addr), msg->rccIam.rtpData[i].port,
				msg->rccIam.rtpData[i].payload, msg->rccIam.rtpData[i].rate);
			rtpDataList.push_back(rtpData);
		}
		cb->onMessageIam(callNum, rtpDataList);

		break;
	}
	case RccMessage::RCC_ANM:
	{
		if (msg->rccAnm.rtpCount > 8 || msg->rccAnm.rtpCount == 0)
		{
			cb->onInvalidMessage(msg);
			break;
		}

		RccRtpDataList rtpDataList;
		for (int i = 0; i<msg->rccAnm.rtpCount; ++i)
		{
			struct in_addr addr;
			addr.S_un.S_addr = htonl(msg->rccAnm.rtpData[i].ip);
			RccRtpData rtpData(inet_ntoa(addr), msg->rccAnm.rtpData[i].port,
				msg->rccAnm.rtpData[i].payload, msg->rccAnm.rtpData[i].rate);
			rtpDataList.push_back(rtpData);
		}

		cb->onMessageAnm(rtpDataList);

		break;
	}
	default:
		cb->onMessage((RccMessage::MessageType)msg->type);

	}
}

void RccUserAgent::getAndDispatchMessage(IRccMessageCallback * cb)
{
	char data[RccMessage::ALLOC_SIZE];
	int sz = RccMessage::ALLOC_SIZE;
	memset(data, 0, RccMessage::ALLOC_SIZE);

	if (getMessage(data, sz) <= 0)
		return;

	dispatchMessage(data, cb);
}
