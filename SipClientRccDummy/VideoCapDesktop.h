#pragma once

#include "VideoEncode.h"
#include "rutil\ThreadIf.hxx"

class VideoCapDesktop : public resip::ThreadIf
{
public:
	VideoCapDesktop(IVideoEncodecCallback* cb);
	~VideoCapDesktop();

	bool start(int width, int height, int rate, int fps = 15, AVCodecID codec = AV_CODEC_ID_H264);
	void stop();

private:
	IVideoEncodecCallback* cb_;
	POINT startPt_;
	int width_;
	int height_;
	int fps_; 
	VideoEncode encode_;

	// Í¨¹ý ThreadIf ¼Ì³Ð
	virtual void thread() override;

};

