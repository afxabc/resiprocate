#pragma once


#include "VideoDecode.h"
#include "vfw.h"

#include "buffer.h"
#include "queue.h"
#include "rutil\ThreadIf.hxx"

class VideoDraw : public IVideoDecodecCallback, resip::ThreadIf
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
	Queue<Buffer> decQueue_;

	// 通过 IVideoDecodecCallback 继承
	virtual void onVideoDecodeFin(char * data, int len, int width, int height, AVPixelFormat fmt, unsigned long tm) override;

	// 通过 ThreadIf 继承
	virtual void thread() override;
};

