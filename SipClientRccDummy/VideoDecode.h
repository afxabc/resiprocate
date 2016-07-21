#pragma once

extern "C"
{
#include "stdint.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
};

class IVideoDecodecCallback
{
public:
	virtual void onVideoDecodeFin(char* data, int len, int width, int height, AVPixelFormat fmt, unsigned long tm) = 0;
};

class VideoDecode
{
public:
	VideoDecode();
	~VideoDecode();

	bool open(int rate, AVCodecID codec = AV_CODEC_ID_H264);
	void close();
	bool decode(const char* data, int len, IVideoDecodecCallback* cb);

private:
	AVCodecContext *pContext_;
	SwsContext *sContext_;
	AVFrame *pFrameRGB_;
	uint8_t* pDataRGB_;
	AVFrame *pFrameYUV_;
	uint8_t* pDataYUV_;
};

