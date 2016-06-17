

#define VIDEOTEST


//#include "str_utils.h"
#include "rutil/Logger.hxx"
#include "processor.h"
#include "mediacontext.h"
#include "ProcessorChain.h"
#include "winvideo.h"
#include "pixconv.h"
#include "sizeconv.h"
#include "tee.h"
#include "videoenc.h"
#include "videodec.h"
#include "videoout.h"
#include "msticker.h"
#include <string>
#include "VideoStream.h"
#include <fstream>

#include "videofile.h"


#ifndef VIDEOTEST
#include "wincardread.h"
#include "wincardwrite.h"
#include "audiodecode.h"
#include "audioencode.h"
#endif

#define RESIPROCATE_SUBSYSTEM Subsystem::APP
using namespace std;

#ifdef VIDEOTEST
int vidiotest()
{
#ifdef WIN32
  /* Initializing windows socket library */
  {
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD (1, 1);
    int i = WSAStartup (wVersionRequested, &wsaData);
    if (i != 0)
      {
		return -1;
        /* return -1; It might be already initilized?? */
      }
  }
#endif

//	Log::initialize("cout","NONE", 0);
	MSVideoSize vsize;
	vsize.width=352;
	vsize.height=288;
	float fps=30;
	MSPixFmt format=MS_RGB24;

	MediaContext mc;
	qinit(&mc.payload0);
	qinit(&mc.payload1);

	Log::initialize("cout", "WARNING", 0);

	MSTicker *tick=new MSTicker(90);

	ProcessorChain *pro=new ProcessorChain();

//	Videofile* winv = new Videofile(tick);
	
	WinVideo* winv = new WinVideo(tick);
	
	PixConv* pix = new PixConv();
	SizeConv* si = new SizeConv(tick);
	Tee* te = new Tee();
	VideoEnc* enc = new VideoEnc(CODEC_ID_H263);
	enc->enc_set_br(256000);
	enc->enc_get_vsize((MSVideoSize*)&vsize);
	enc->enc_get_fps((float*)&fps);
	enc->enc_set_tick(tick);
//	InfoLog(<<"Setting vsize="<<vsize.width<<"*"<<vsize.height<<", fps=%f"<<fps);

	winv->v4w_set_vsize(vsize);
	winv->v4w_set_fps(fps);
	winv->v4w_get_pix_fmt((MSPixFmt*)&format);
	pix->pixconv_set_pixfmt(format);
	pix->pixconv_set_vsize(vsize);
	si->sizeconv_set_vsize(vsize);
	si->sizeconv_set_fps(fps);

	winv->v4w_preprocess();
	winv->v4w_set_fps(fps);
	enc->enc_preprocess();

	VideoDec* dec = new VideoDec(CODEC_ID_H263);
	dec->dec_preprocess();
	VideoOut* vio = new VideoOut();

	vio->video_out_set_vsize(vsize);
	vio->video_out_preprocess();

	RTPSession *session;
	RtpWrite* rtpwrite;
	RtpRead* rtpread;

	session = new RTPSession();
	session->Create(9008);
	session->AddDestination(0xc0a8011c, 9008);
	rtpwrite = new RtpWrite(session);
	rtpread = new RtpRead(session);

	pro->addProcessor(std::auto_ptr<Processor>(winv));
	pro->addProcessor(std::auto_ptr<Processor>(pix));
	pro->addProcessor(std::auto_ptr<Processor>(si));
	pro->addProcessor(std::auto_ptr<Processor>(te));
	pro->addProcessor(std::auto_ptr<Processor>(enc));
//	pro->addProcessor(std::auto_ptr<Processor>(rtpwrite));
//	pro->addProcessor(std::auto_ptr<Processor>(rtpread));
	pro->addProcessor(std::auto_ptr<Processor>(dec));
	pro->addProcessor(std::auto_ptr<Processor>(vio));

	tick->executechain.addProcessor(std::auto_ptr<Processor>(pro));
	tick->run();
/*
	ofstream file("videofile", ios::out | ios::binary);
	int cc = 3000;*/
	while(1)
	{
#ifdef WIN32
		MSG msg;

		Sleep(10);
		while (PeekMessage(&msg, NULL, 0, 0,1))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

#endif
	}
	
	tick->shutdown();
	tick->join();
/*
	mblk_t *m;
	while((m=getq(&winv->rq))!=NULL)
	{
		file.write((char*)m->b_rptr, m->b_wptr-m->b_rptr);
		printf("\n len = %d", m->b_wptr-m->b_rptr);
	}
	file.close();

	InfoLog(<<"delete ticker");
	delete tick;
*/
//	delete vio;
//	delete dec;
//	delete enc;
//	deletewinv;
//	delete pix;
//	delete si;
	return 0;
/*
	VideoStream *stream = new VideoStream();
	stream->create_rtpsession(8008);
	stream->videostream_start("192.168.1.2",9008,98,false,true);

	while(1)
	{
		string command;
		string value;

		cin>>command;
		if(command==string("quit"))
		{
			break;
		}
	}
	stream->videostream_stop();
	delete stream;*/
}

#else

int audiotest()
{
//	Log::initialize("cout", "INFO", argv[0]);

	MSTicker *tick=new MSTicker(20);
	ProcessorChain *pro=new ProcessorChain();

	WincardRead* cardread = new WincardRead(0);
	cardread->card_init();
//	cardread->card_detect();
	cardread->card_read_preprocess();
	pro->addProcessor(std::auto_ptr<Processor>(cardread));
	AudioEncode* encode = new AudioEncode(G729);
	pro->addProcessor(std::auto_ptr<Processor>(encode));
	
	AudioDecode* decode = new AudioDecode();
	pro->addProcessor(std::auto_ptr<Processor>(decode));
	WincardWrite* cardwrite = new WincardWrite(0);
//	MSSndCard *mscard=new MSSndCard();
	WincardWrite::card_detect();
	cardwrite->card_init();
	cardwrite->cardwrite_preprocess();
	
	pro->addProcessor(std::auto_ptr<Processor>(cardwrite));

	tick->executechain.addProcessor(std::auto_ptr<Processor>(pro));
	
	tick->run();
	WincardWrite* card1=new WincardWrite(0);
//	AudioStream stream;
//	stream.ringstream_start("Receive.wav",0);

//	testfun();
//	Sleep(2000);
//	stream.ringstream_start("toy.wav",0);
//	testfun();
	
	while(1)
	{
		Sleep(100);
	}
	return 0;
}

#endif


int main(int argc, char** argv)
{
#ifdef VIDEOTEST
	vidiotest();
#else
	audiotest();
#endif
}
