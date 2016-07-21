#include "stdafx.h"
#include "VideoEncode.h"


VideoEncode::VideoEncode()
	: pContext_(NULL)
	, pCodec_(NULL)
	, sContext_(NULL)
	, pFrameRGB_(NULL)
	, pFrameYUV_(NULL)
	, pDataYUV_(NULL)
	, nTimestamp_(0)
	, pt_(RTP_PAYLOAD_H264)
{
	avcodec_register_all();
}

VideoEncode::~VideoEncode()
{
	close();
}

bool VideoEncode::open(int width, int height, int rate, int fps, AVCodecID codec)
{
	close();

	//Ö»Ö§³ÖH263, H264
	if (AV_CODEC_ID_H264 == codec)
		pt_ = RTP_PAYLOAD_H264;
	else if (AV_CODEC_ID_H263 == codec)
		pt_ = RTP_PAYLOAD_H263;
	else return false;

	pCodec_ = avcodec_find_encoder(codec);
	if (pCodec_ == NULL)
		return false;

	pContext_ = avcodec_alloc_context3(pCodec_);

	pContext_->width = width;
	pContext_->height = height;
	pContext_->time_base.num = 1;
	pContext_->time_base.den = fps;
	pContext_->gop_size = pContext_->time_base.den * 3; // emit one intra frame every 10 second 
	pContext_->bit_rate = rate; // put sample parameters 
	pContext_->bit_rate_tolerance = (float)pContext_->bit_rate / (pContext_->time_base.den - 1); // put sample parameters 
	pContext_->pix_fmt = AV_PIX_FMT_USE;
	pContext_->max_b_frames = 0;
	pContext_->codec_type = AVMEDIA_TYPE_VIDEO;
	pContext_->qmin = 10;
	pContext_->qmax = 50;
	pContext_->pre_me = 2;
	pContext_->lmin = 1;
	pContext_->lmax = 5;
	pContext_->flags |= CODEC_FLAG_TRUNCATED;

	nTimestamp_ = 0;
	pFrameRGB_ = av_frame_alloc();
	pFrameYUV_ = av_frame_alloc();
	pDataYUV_ = new uint8_t[avpicture_get_size(AV_PIX_FMT_USE, width, height)];
	avpicture_fill((AVPicture *)pFrameYUV_, (const uint8_t*)pDataYUV_, AV_PIX_FMT_USE, width, height);

	if (avcodec_open2(pContext_, pCodec_, NULL) != 0)
	{
		close();
		return false;
	}

	return true;
}

void VideoEncode::close()
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

	if (pDataYUV_ != NULL)
	{
		delete[] pDataYUV_;
		pDataYUV_ = NULL;
	}
}

bool VideoEncode::encode(const char* data, int len, IVideoEncodecCallback * cb, AVPixelFormat fmt)
{
	if (pContext_ == NULL || cb == NULL || data == NULL)
		return false;

	int width = pContext_->width;
	int height = pContext_->height;

	if (sContext_ == NULL)
		sContext_ = sws_getContext(width, height, fmt, width, height, AV_PIX_FMT_USE, SWS_BICUBIC, NULL, NULL, NULL);

	avpicture_fill((AVPicture *)pFrameRGB_, (const uint8_t*)data, fmt, width, height);
	int h = sws_scale(sContext_, pFrameRGB_->data, pFrameRGB_->linesize, 0, height, pFrameYUV_->data, pFrameYUV_->linesize);

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	int got_frm = 0;
	len = avcodec_encode_video2(pContext_, &pkt, pFrameYUV_, &got_frm);
	if (got_frm)
	{
		cb->onVideoEncodeFin((char*)pkt.data, pkt.size, pt_, (pkt.flags==AV_PKT_FLAG_KEY), 1000 / pContext_->time_base.den);
		av_packet_unref(&pkt);
		return true;
	}

	return false;
}
