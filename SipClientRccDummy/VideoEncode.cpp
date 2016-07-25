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

	//只支持H263, H264
	if (AV_CODEC_ID_H264 == codec)
		pt_ = RTP_PAYLOAD_H264;
	else if (AV_CODEC_ID_H263 == codec)
		pt_ = RTP_PAYLOAD_H263;
	else return false;

	pCodec_ = avcodec_find_encoder(codec);
	if (pCodec_ == NULL)
		return false;

	pContext_ = avcodec_alloc_context3(pCodec_);

	av_opt_set(pContext_->priv_data, "preset", "superfast", 0); 
	av_opt_set(pContext_->priv_data, "tune", "zerolatency", 0);

	pContext_->codec_type = AVMEDIA_TYPE_VIDEO;
	pContext_->pix_fmt = AV_PIX_FMT_USE;
	pContext_->flags |= CODEC_FLAG_TRUNCATED;
	pContext_->width = width;
	pContext_->height = height;
	pContext_->time_base.num = 1;
	pContext_->time_base.den = fps;
	pContext_->max_b_frames = 0;
	pContext_->gop_size = fps;
	pContext_->bit_rate_tolerance = (float)pContext_->bit_rate / (50); 
	pContext_->coder_type = FF_CODER_TYPE_VLC;
	pContext_->me_method = 7; //motion estimation algorithm
	pContext_->me_subpel_quality = 4;

	//设置恒定码率
	pContext_->bit_rate = rate*8; 
//	pContext_->rc_max_rate = pContext_->bit_rate;
//	pContext_->rc_min_rate = pContext_->bit_rate;
	pContext_->bit_rate_tolerance = pContext_->bit_rate;
	
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
		char* pdata = (char*)pkt.data;
		int size = pkt.size;
		if (pdata[0] == 0 && pdata[1] == 0)
		{
			if (pdata[2] == 0 && pdata[3] == 1)
				pdata += 4, size -= 4;
			else if (pdata[2] == 1)
				pdata += 3, size -= 3;
		}

		static const int MAX_PACKET_SIZE = 1024;
		if (size > MAX_PACKET_SIZE)
		{
			char outBuf[MAX_PACKET_SIZE];
			outBuf[0] = (pdata[0] & 0xE0) | 28; // FU indicator  
			char FU = (pdata[0] & 0x1F); // FU header (with S bit)  
			pdata += 1;
			size -= 1;
			bool first = true;
			unsigned long tm = 0;
			while (size > 0)
			{
				int len = (size > (MAX_PACKET_SIZE - 2)) ? (MAX_PACKET_SIZE - 2) : size;
				memcpy(outBuf + 2, pdata, len);
				if (first)
				{
					outBuf[1] = FU | 0x80;
					first = false;
				}
				else
				{
					outBuf[1] = FU;
					if (len == size)		//last
					{
						outBuf[1] = FU | 0x40;
						tm = 1000 / pContext_->time_base.den;
					}
				}
				cb->onVideoEncodeFin(outBuf, len+2, pt_, 1, tm);

				pdata += len;
				size -= len;
			}
		}
		else cb->onVideoEncodeFin(pdata, size, pt_, 1, 1000 / pContext_->time_base.den);
		av_packet_unref(&pkt);
		return true;
	}

	return false;
}
