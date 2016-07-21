#pragma once


#include "VideoDecode.h"
#include "vfw.h"

class VideoDraw : public IVideoDecodecCallback
{
public:
	VideoDraw();
	~VideoDraw();

	bool start(CWnd* drawWnd, int rate, AVCodecID codec = AV_CODEC_ID_H264);
	void stop();
	bool decodeAndDraw(const char* data, int len);

private:
	void resetDraw(int width, int height);

private:
	VideoDecode decode_;
	CWnd* drawWnd_;
	BITMAPINFOHEADER drawInfo_;
	HDRAWDIB drawDib_;
	RECT drawRect_;

	// Í¨¹ý IVideoDecodecCallback ¼Ì³Ð
	virtual void onVideoDecodeFin(char * data, int len, int width, int height, AVPixelFormat fmt, unsigned long tm) override;
};

