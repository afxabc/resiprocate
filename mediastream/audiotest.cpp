

#include "mediastream/MSTicker.h"
#include "mediastream/processor.h"
#include "mediastream/mediacontext.h"
#include "mediastream/ProcessorChain.h"
#include "mediastream/WincardRead.h"
#include "mediastream/WincardWrite.h"
#include "mediastream/AudioDecode.h"
#include "mediastream/AudioEncode.h"
#include "mediastream/rtpread.h"
#include "mediastream/rtpwrite.h"
#include "mediastream/Dtmf.h"

//#include "mediastream/videodec.h"
//#include "mediastream/videoout.h"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP


int main(int argc, char** argv)
{
//	MSVideoSize vsize;
//	float fps;
//	MSPixFmt format;
	initNetwork();
	MediaContext mc;
	qinit(&mc.payload0);
	qinit(&mc.payload1);
 
	Log::initialize("cout", "NONE", argv[0]);

	MSTicker *tick=new MSTicker(mc, 20);

#if 0
	ProcessorChain *pro=new ProcessorChain();

	WincardRead* cardread = new WincardRead();
	cardread->card_init();
	cardread->card_detect();
	cardread->card_read_preprocess();
	pro->addProcessor(std::auto_ptr<Processor>(cardread));
	AudioEncode* encode = new AudioEncode(iLBC);
	pro->addProcessor(std::auto_ptr<Processor>(encode));

	AudioDecode* decode = new AudioDecode();
	pro->addProcessor(std::auto_ptr<Processor>(decode));
	WinscardWrite* cardwrite = new WinscardWrite();
	cardwrite->card_init();
	cardwrite->card_detect();
	cardwrite->cardwrite_preprocess();
	pro->addProcessor(std::auto_ptr<Processor>(cardwrite));
#endif

	//test dtmf
	RTPSession *send_session = new RTPSession(); 
	int status = send_session->Create(5000);
	send_session->AddDestination(0xc0a8011c, 5002);
	RTPSession *recv_session = new RTPSession();
	status = recv_session->Create(5002);

	ProcessorChain* sendprocess = new ProcessorChain();

	WincardRead* cardread = new WincardRead();
	cardread->card_init();
	cardread->card_detect();
	cardread->card_read_preprocess();
	sendprocess->addProcessor(std::auto_ptr<Processor>(cardread));

	RtpWrite* rtpwrite = new RtpWrite(send_session);
	sendprocess->addProcessor(std::auto_ptr<Processor>(rtpwrite));


	RtpRead* rtpread = new RtpRead(recv_session);
	sendprocess->addProcessor(std::auto_ptr<Processor>(rtpread));

	Dtmf* dtmf = new Dtmf();
	sendprocess->addProcessor(std::auto_ptr<Processor>(dtmf));

	WinscardWrite* cardwrite = new WinscardWrite();
	cardwrite->card_init();
	cardwrite->card_detect();
	cardwrite->cardwrite_preprocess();
	sendprocess->addProcessor(std::auto_ptr<Processor>(cardwrite));
	tick->executechain.addProcessor(std::auto_ptr<Processor>(sendprocess));
	//end of test

//	tick->executechain.addProcessor(std::auto_ptr<Processor>(sendprocess));
	
	tick->run();
	
	int i=0;
	while(1)
	{
		char tmp[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#', 'a', 'b', 'c', 'd'};
		
//		Dtmf::Dtmf_send(send_session, tmp[i++]);
		i%=16;
		Sleep(1000);
	}
	return 0;
}