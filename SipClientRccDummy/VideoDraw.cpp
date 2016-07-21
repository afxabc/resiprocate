#include "stdafx.h"
#include "VideoDraw.h"


VideoDraw::VideoDraw()
	: drawDib_(NULL)
	, drawWnd_(NULL)
{
	memset(&drawInfo_, 0, sizeof(drawInfo_));
	drawInfo_.biSize = sizeof(drawInfo_);
}


VideoDraw::~VideoDraw()
{
	stop();
}

bool VideoDraw::start(CWnd * drawWnd, int rate, AVCodecID codec)
{
	drawWnd_ = drawWnd;
	return decode_.open(rate, codec);
}

void VideoDraw::stop()
{
	decode_.close();
	if (drawDib_ != NULL)
	{
		::DrawDibEnd(drawDib_);
		::DrawDibClose(drawDib_);
		drawDib_ = NULL;
	}
	drawWnd_ = NULL;
}

bool VideoDraw::decodeAndDraw(const char * data, int len)
{
	return decode_.decode(data, len, this);
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
