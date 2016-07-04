#include "inetaddress.h"

InetAddress InetAddress::NULL_ADDR(0);

InetAddress::InetAddress(unsigned short port)
{
	update("", port);
}

InetAddress::InetAddress(const std::string& ip, unsigned short port)
{
	update(ip, port);
}

InetAddress::~InetAddress(void)
{
}

void InetAddress::update(unsigned short port)
{

	memset((char*) &(sockaddr_), 0, sizeof((sockaddr_)));

	sockaddr_.sin_family = AF_INET;
	sockaddr_.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr_.sin_port = htons(port);
	
}

void InetAddress::update(const std::string& ip, unsigned short port)
{
	memset((char*) &(sockaddr_), 0, sizeof((sockaddr_)));

	sockaddr_.sin_family = AF_INET;

	if (ip.length() > 1)
		sockaddr_.sin_addr.s_addr = inet_addr(ip.c_str());
	else sockaddr_.sin_addr.s_addr = htonl(INADDR_ANY);

	sockaddr_.sin_port = htons(port);
}

void InetAddress::update(const std::string& str)		//eg: 127.0.0.1:5555
{
	std::string::size_type index = str.find_first_of(':');
	if (index == std::string::npos)
		return;

	std::string strPort = str.substr(index+1);
	unsigned short port = atoi(strPort.c_str());
	if (port == 0)
		return;

	std::string strIP = str.substr(0, index);

	memset((char*) &(sockaddr_), 0, sizeof((sockaddr_)));
	sockaddr_.sin_family = AF_INET;
	sockaddr_.sin_port = htons(port);
	sockaddr_.sin_addr.s_addr = inet_addr(strIP.c_str());
}

int InetAddress::toString(char* buf, int size) const
{
	BYTE* pb = (BYTE*)(&(sockaddr_.sin_addr));
	unsigned short port = ntohs(sockaddr_.sin_port);
	
	return snprintf(buf, size, "%d.%d.%d.%d:%u", pb[0], pb[1], pb[2], pb[3], port);
}

std::string InetAddress::toString() const
{
	static const int BUFF_SIZE = 64;
	char buf[BUFF_SIZE+1];

	int len = toString(buf, BUFF_SIZE);
	buf[len] = 0;

	return buf;
}

std::string InetAddress::getIP() const
{
	static const int BUFF_SIZE = 64;
	char buf[BUFF_SIZE+1];

	BYTE* pb = (BYTE*)(&(sockaddr_.sin_addr));
	int len = snprintf(buf, BUFF_SIZE, "%d.%d.%d.%d", pb[0], pb[1], pb[2], pb[3]);
	buf[len] = 0;

	return buf;
}
