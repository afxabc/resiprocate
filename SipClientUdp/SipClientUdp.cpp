// SipClientUdp.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <signal.h>
#include <iostream>
#include <rutil/Log.hxx>
#include "basicClientUserAgent.hxx"

//SIP���ò���
static const char* sipHost = "10.10.3.100";		//sip��������ַ
static const char* sipPasswd = "1234";
static unsigned short sipPort = 12000;			//����sip�˿�
//RCC����˿�
static unsigned short rccPort = RCC_SERVER_PORT_BASE;
static const char * rccIP = NULL;

static resip::BasicClientUserAgent* agent = NULL;
static bool exitSignalDetected = false;

static void signalHandler(int signo)
{
	std::cerr << "Shutting down" << std::endl;
	exitSignalDetected = true;
}

int main()
{
#ifndef _WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		std::cerr << "Couldn't install signal handler for SIGPIPE" << std::endl;
		exit(-1);
	}
#endif

	if (::signal(SIGABRT, signalHandler) == SIG_ERR)
	{
		std::cerr << "Couldn't install signal handler for SIGABRT" << std::endl;
		exit(-1);
	}

	if (::signal(SIGINT, signalHandler) == SIG_ERR)
	{
		std::cerr << "Couldn't install signal handler for SIGINT" << std::endl;
		exit(-1);
	}

	if (::signal(SIGTERM, signalHandler) == SIG_ERR)
	{
		std::cerr << "Couldn't install signal handler for SIGTERM" << std::endl;
		exit(-1);
	}

	resip::Log::initialize("cout", "INFO", "<<<SipClientUdp>>>");
	// Initialize network
	resip::initNetwork();

	agent = new resip::BasicClientUserAgent();
	if (!agent->start(sipHost, sipPasswd, rccPort, rccIP, sipPort))
	{
		std::cerr << "Couldn't start sipAgent" << std::endl;
		exit(-1);
	}

	while (!exitSignalDetected)
	{
#ifdef WIN32
		if (getc(stdin) == 'q')
			break;
#else
		usleep(100 * 1000);
#endif
	}

	if (agent != NULL)
	{
		agent->stop();
		delete agent;
		agent = NULL;
	}

#ifdef WIN32
//	system("pause");
#endif

    return 0;
}

