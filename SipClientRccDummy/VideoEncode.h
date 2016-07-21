#pragma once

extern "C"
{
#include "stdint.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
};

class IVideoEncodecCallback
{
public:
	virtual void onVideoEncodeFin(char* data, int len, unsigned char pt, bool mark, unsigned long tm) = 0;
};

#define RTP_PAYLOAD_H263 34
#define RTP_PAYLOAD_H264 96

class VideoEncode
{
public:
	VideoEncode();
	~VideoEncode();

	bool open(int width, int height, int rate, int fps = 15, AVCodecID codec = AV_CODEC_ID_H264);
	void close();
	bool encode(const char* data, int len, IVideoEncodecCallback* cb, AVPixelFormat fmt = AV_PIX_FMT_BGRA);

private:
	static const AVPixelFormat AV_PIX_FMT_USE = AV_PIX_FMT_YUV420P;
	AVCodecContext *pContext_;
	AVCodec* pCodec_;
	SwsContext *sContext_;
	AVFrame *pFrameRGB_;
	AVFrame *pFrameYUV_;
	uint8_t* pDataYUV_;
	unsigned long nTimestamp_;
	unsigned char pt_;

};

