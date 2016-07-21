#include "stdafx.h"
#include "VideoDecode.h"


VideoDecode::VideoDecode()
	: pContext_(NULL)
	, sContext_(NULL)
	, pFrameRGB_(NULL)
	, pDataRGB_(NULL)
	, pFrameYUV_(NULL)
	, pDataYUV_(NULL)
{
	avcodec_register_all();
}


VideoDecode::~VideoDecode()
{
	close();
}

bool VideoDecode::open(int rate, AVCodecID codec)
{
	close();

	AVCodec* avc = avcodec_find_decoder(codec);
	if (avc == NULL)
		return false;

	pContext_ = avcodec_alloc_context3(avc);

	pContext_->bit_rate = rate; 
	pContext_->gop_size = 10;
	pContext_->max_b_frames = 1;
	pContext_->pix_fmt = AV_PIX_FMT_YUV420P;
	pContext_->flags |= CODEC_FLAG_TRUNCATED; 

	pFrameRGB_ = av_frame_alloc();
	pFrameYUV_ = av_frame_alloc();

	if (avcodec_open2(pContext_, avc, NULL) < 0)
	{
		close();
		return false;
	}
	return true;
}

void VideoDecode::close()
{
	if (pContext_)
	{
		avcodec_close(pContext_);
		av_free(pContext_);
		pContext_ = NULL;
	}

	if (sContext_ != NULL)
	{
		sws_freeContext(sContext_);
		sContext_ = NULL;
	}

	if (pFrameRGB_ != NULL)
	{
		av_free(pFrameRGB_);
		pFrameRGB_ = NULL;
	}

	if (pFrameYUV_ != NULL)
	{
		av_free(pFrameYUV_);
		pFrameYUV_ = NULL;
	}

	if (pDataRGB_ != NULL)
	{
		delete[] pDataRGB_;
		pDataRGB_ = NULL;
	}

	if (pDataYUV_ != NULL)
	{
		delete[] pDataYUV_;
		pDataYUV_ = NULL;
	}

}

bool VideoDecode::decode(const char * data, int len, IVideoDecodecCallback * cb)
{
	if (pContext_ == NULL || cb == NULL || data == NULL)
		return false;

	AVPacket pkt;
	int size = len;
	const char * pb = data;
	while (size > 0)
	{
		pkt.data = (uint8_t*)pb;
		pkt.size = size;

		int got_picture = 0;
		int len = avcodec_decode_video2(pContext_, pFrameYUV_, &got_picture, &pkt);
		if (len < 0)
			break;

		if (got_picture > 0)
		{
			int width = pFrameYUV_->width;
			int height = pFrameYUV_->height; 
			AVPixelFormat GDI_FORMAT = AV_PIX_FMT_BGRA;
			if (pDataRGB_ == NULL)
			{
				pDataRGB_ = new uint8_t[avpicture_get_size(GDI_FORMAT, width, height)];
				avpicture_fill((AVPicture *)pFrameRGB_, pDataRGB_, GDI_FORMAT, width, height);
			}
			if (sContext_ == NULL)
				sContext_ = sws_getContext(width, height, (AVPixelFormat)pFrameYUV_->format, width, height, GDI_FORMAT, SWS_BICUBIC, NULL, NULL, NULL);
			int h = sws_scale(sContext_, pFrameYUV_->data, pFrameYUV_->linesize, 0, height, pFrameRGB_->data, pFrameRGB_->linesize);

			cb->onVideoDecodeFin((char*)pFrameRGB_->data[0], pFrameRGB_->linesize[0]*height, width, height, GDI_FORMAT, pkt.dts);
		}

		size -= len;
		pb += len;
	}
	return false;
}
