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

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

int main(int argc, char** argv)
{
	MSVideoSize vsize;
	float fps;
	MSPixFmt format;

	MediaContext mc;
	qinit(&mc.payload0);
	qinit(&mc.payload1);

	Log::initialize("cout", "WARNING", argv[0]);

	MSTicker *tick=new MSTicker(mc, 20);
	ProcessorChain *pro=new ProcessorChain();

	WinVideo* winv = new WinVideo(tick);
	PixConv* pix = new PixConv();
	SizeConv* si = new SizeConv(tick);
	Tee* te = new Tee();
	VideoEnc* enc = new VideoEnc(CODEC_ID_H263);
	enc->enc_get_vsize((MSVideoSize*)&vsize);
	enc->enc_get_fps((float*)&fps);
	enc->enc_set_tick(tick);
	InfoLog(<<"Setting vsize="<<vsize.width<<"*"<<vsize.height<<", fps=%f"<<fps);

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

	pro->addProcessor(std::auto_ptr<Processor>(winv));
	pro->addProcessor(std::auto_ptr<Processor>(pix));
	pro->addProcessor(std::auto_ptr<Processor>(si));
	pro->addProcessor(std::auto_ptr<Processor>(te));
	pro->addProcessor(std::auto_ptr<Processor>(enc));
	pro->addProcessor(std::auto_ptr<Processor>(dec));
	pro->addProcessor(std::auto_ptr<Processor>(vio));

	tick->executechain.addProcessor(std::auto_ptr<Processor>(pro));
	tick->run();
	while(1)
	{
		Sleep(100);
	}
	return 0;
}