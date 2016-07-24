#include "stdafx.h"
#include "VideoDraw.h"


VideoDraw::VideoDraw()
	: drawDib_(NULL)
	, drawWnd_(NULL)
{
	memset(&drawInfo_, 0, sizeof(drawInfo_));
	drawInfo_.biSize = sizeof(drawInfo_);
	mShutdown = true;
}


VideoDraw::~VideoDraw()
{
	stop();
}

bool VideoDraw::start(CWnd * drawWnd, int rate, AVCodecID codec)
{
	drawWnd_ = drawWnd;
	if (!decode_.open(rate, codec))
		return false;
	this->run();
	return true;
}

void VideoDraw::stop()
{
	if (mShutdown)
		return;

	mShutdown = true;
	this->shutdown();
	this->join();

	decode_.close();
	if (drawDib_ != NULL)
	{
		::DrawDibEnd(drawDib_);
		::DrawDibClose(drawDib_);
		drawDib_ = NULL;
	}

	if (drawWnd_ != NULL)
	{
//		drawWnd_->Invalidate();
		drawWnd_ = NULL;
	}
		
}

bool VideoDraw::decodeAndDraw(const char * data, int len)
{
	decQueue_.putBack(Buffer(data, len));
	return true;
//	return decode_.decode(data, len, this);
}

void VideoDraw::resetDraw(int width, int height)
{
	drawInfo_.biWidth = width;
	drawInfo_.biHeight = height;
	drawInfo_.biCompression = BI_RGB;
	drawInfo_.biBitCount = 32;
	drawInfo_.biPlanes = 1;

	RECT r;
	drawWnd_->GetClientRect(&r);
	drawRect_.left = (r.right - drawInfo_.biWidth) / 2;
	drawRect_.right = drawRect_.left + drawInfo_.biWidth;
	drawRect_.top = (r.bottom - drawInfo_.biHeight) / 2;
	drawRect_.bottom = drawRect_.top + drawInfo_.biHeight;


	if (drawDib_ != NULL)
	{
		::DrawDibEnd(drawDib_);
		::DrawDibClose(drawDib_);
	}
	drawDib_ = ::DrawDibOpen();

	CClientDC dc(drawWnd_);
	DrawDibBegin(drawDib_, dc.m_hDC, drawInfo_.biWidth, drawInfo_.biHeight, &drawInfo_, drawInfo_.biWidth, drawInfo_.biHeight, DDF_JUSTDRAWIT);

}

void VideoDraw::onVideoDecodeFin(char * data, int len, int width, int height, AVPixelFormat fmt, unsigned long tm)
{
	if (drawWnd_ == NULL)
		return;

	if (drawDib_ == NULL || drawInfo_.biWidth != width || drawInfo_.biHeight != height)
		resetDraw(width, height);

	CClientDC dc(drawWnd_);
	DrawDibDraw(drawDib_, dc.m_hDC, drawRect_.left, drawRect_.top, drawInfo_.biWidth, drawInfo_.biHeight, &drawInfo_, data, 0, 0, drawInfo_.biWidth, drawInfo_.biHeight, DDF_JUSTDRAWIT);
}

void VideoDraw::thread()
{
	Buffer tmp;
	mShutdown = false;
	while (!isShutdown())
	{
		if (decQueue_.getFront(tmp, 100))
		{
			decode_.decode(tmp.beginRead(), tmp.readableBytes(), this);
		}
	}
}
