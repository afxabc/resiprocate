#include "VideoCapDesktop.h"

#include "timestamp.h"

VideoCapDesktop::VideoCapDesktop(IVideoEncodecCallback* cb)
	: cb_(cb)
	, width_(100)
	, height_(100)
	, fps_(15)
{
	mShutdown = true;
}

VideoCapDesktop::~VideoCapDesktop()
{
	stop();
}

bool VideoCapDesktop::start(int width, int height, int rate, int fps, AVCodecID codec)
{
	stop();

	width_ = (width < 10) ? 10 : width;
	height_ = (height < 10) ? 10 : height;
	fps_ = (fps < 1) ? 1 : fps;
	fps_ = (fps >100) ? 100 : fps;

	if (!encode_.open(width, height, rate, fps))
		return false;

	this->run();

	return true;
}

void VideoCapDesktop::stop()
{
	if (mShutdown)
		return;

	mShutdown = true;
	this->shutdown();
	this->join();

	encode_.close();
}

void VideoCapDesktop::thread()
{
	int imgSize = width_*height_ * 4;
	unsigned char* data = new unsigned char[imgSize];

	BITMAPINFOHEADER imgInfo;
	ZeroMemory(&imgInfo, sizeof(imgInfo));
	imgInfo.biSize = sizeof(BITMAPINFOHEADER);
	imgInfo.biWidth = width_;
	imgInfo.biHeight = height_;
	imgInfo.biCompression = BI_RGB;
	imgInfo.biBitCount = 32;
	imgInfo.biPlanes = 1;

	MicroSecond delay(1000 / fps_);

	HDC hdc = CreateDCA("DISPLAY", NULL, NULL, NULL);
	HDC hdc2 = ::CreateCompatibleDC(hdc);
	HBITMAP hbmp = ::CreateCompatibleBitmap(hdc, width_, height_);
	::SelectObject(hdc2, hbmp);

	int WIDTH = GetSystemMetrics(SM_CXSCREEN);
	int HEIGHT = GetSystemMetrics(SM_CYSCREEN);

	mShutdown = false;
	while (!isShutdown())
	{
		Timestamp now = Timestamp::NOW();

		POINT pt;
		::GetCursorPos(&pt);
		int dx = pt.x*width_ / WIDTH;
		int dy = pt.y*height_ / HEIGHT;
		::BitBlt(hdc2, 0, 0, width_, height_, hdc, pt.x-dx, pt.y-dy, SRCCOPY);
		int len = ::GetDIBits(hdc2, hbmp, 0, height_, data, (LPBITMAPINFO)(&imgInfo), DIB_RGB_COLORS);
		if (len > 0)
		{
			encode_.encode((const char*)data, imgInfo.biSizeImage, cb_);
		}

		delay = 1000 / fps_ + now - Timestamp::NOW();
		if (delay > 0)
			Sleep(delay);
	}

	::DeleteDC(hdc);
	::DeleteDC(hdc2);
	::DeleteObject(hbmp);

	delete[] data;
}
