#include "stdafx.h"
#include "AudioWrite.h"
#include "jrtplib\rtpsession.h"
#include "jrtplib\rtppacket.h"


AudioWrite::AudioWrite()
	: lpDSound_(NULL)
	, pDSB8_(NULL)
	, pDSN_(NULL)
	, mute_(false)
{
	HRESULT hr = DirectSoundCreate8(NULL, &lpDSound_, NULL);
	mShutdown = true;
}


AudioWrite::~AudioWrite()
{
	stop();
	lpDSound_->Release();
}

bool AudioWrite::start(UINT rate, RTPSession* session)
{
	pSession_ = session;
	stop();

	HRESULT hr = lpDSound_->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
	if (FAILED(hr))
	{
		TRACE("SetCooperativeLevel failed %X\n", hr);
		return false;
	}
	/*
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.cbSize = 0;//PCM格式时，忽略/
	wfx.nAvgBytesPerSec = 16000;//数据传输速率，字节/秒/
	wfx.nBlockAlign = 2;//每个样本的字节数/
	wfx.nChannels = 1;//通道数/
	wfx.nSamplesPerSec = 8000;//采样率/
	wfx.wBitsPerSample = 16;//每个样本的bit数/
	*/
	fmtWave_.wFormatTag = WAVE_FORMAT_PCM;
	fmtWave_.cbSize = sizeof(fmtWave_);
	fmtWave_.nAvgBytesPerSec = rate*2;
	fmtWave_.nBlockAlign = 2;
	fmtWave_.nChannels = 1;
	fmtWave_.nSamplesPerSec = rate;
	fmtWave_.wBitsPerSample = 16;

	int delay = 40; //100ms缓存
	bufferNotifySize_ = fmtWave_.nAvgBytesPerSec*delay / 1000;

	//创建辅助缓冲区对象
	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags =
		DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFX | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | 
		DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
	dsbd.dwBufferBytes = bufferNotifySize_*MAX_AUDIO_BUF;	//((alaw_)?playSpan_*2:playSpan_); 
	dsbd.lpwfxFormat = &fmtWave_;

	pDSB8_ = NULL;
	pDSN_ = NULL;
	LPDIRECTSOUNDBUFFER pDSB = NULL;

	//获取接口
	bool res = false;
	do
	{
		if (FAILED(hr = lpDSound_->CreateSoundBuffer(&dsbd, &pDSB, NULL)))
		{
			TRACE("CreateSoundBuffer failed %X \n", hr);
			break;
		}
		if (FAILED(hr = pDSB->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&pDSB8_)))
		{
			TRACE("QueryInterface IID_IDirectSoundBuffer8 failed %X\n", hr);
			break;
		}
		if (FAILED(hr = pDSB->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&pDSN_)))
		{
			TRACE("QueryInterface IID_IDirectSoundNotify failed %X\n", hr);
			break;
		}
		res = true;
	} while (0);

	if (pDSB != NULL)
		pDSB->Release();

	if (!res)
	{
		if (pDSB8_ != NULL)
			pDSB8_->Release();
		if (pDSN_ != NULL)
			pDSN_->Release();
		return false;
	}

	this->run();
	return true;
}

void AudioWrite::stop()
{
	if (mShutdown)
		return;

	mShutdown = true;
	this->shutdown();
	this->join();
	playQueue_.clear();
	recvBuff_.erase();
}

void AudioWrite::inputPcm(const char * data, int len)
{
	if (isShutdown() || mute_)
	{
		if (recvBuff_.readableBytes() > 0)
			recvBuff_.erase();
		return;
	}

	int matchSize = bufferNotifySize_;
	recvBuff_.pushBack(data, len, true);
	if (recvBuff_.readableBytes() < matchSize)
		return;

	Buffer tmp;
	BYTE* buf = (BYTE*)recvBuff_.beginRead();
	tmp.pushBack((char*)buf, matchSize);
	playQueue_.putBack(tmp);

	recvBuff_.eraseFront(matchSize);
}

void AudioWrite::thread()
{
	//设置提醒

	DSBPOSITIONNOTIFY DSBPositionNotify[MAX_AUDIO_BUF];
	HANDLE ev[MAX_AUDIO_BUF];
	for (int i = 0; i<MAX_AUDIO_BUF; i++)
	{
		DSBPositionNotify[i].dwOffset = i*bufferNotifySize_;
		ev[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		DSBPositionNotify[i].hEventNotify = ev[i];
	}
	pDSN_->SetNotificationPositions(MAX_AUDIO_BUF, DSBPositionNotify);

	pDSB8_->SetCurrentPosition(0);
	HRESULT hr = pDSB8_->Play(0, 0, DSBPLAY_LOOPING);

	LPVOID buf = NULL;
	DWORD  buf_len = 0;
	DWORD obj = WAIT_OBJECT_0;
	DWORD offset = bufferNotifySize_;
	Buffer tmp;

	mute_ = false;
	mShutdown = false;

	while (!mShutdown)
	{
		obj = WaitForMultipleObjects(MAX_AUDIO_BUF, ev, FALSE, INFINITE);

		if ((obj < WAIT_OBJECT_0) || (obj >= WAIT_OBJECT_0 + MAX_AUDIO_BUF))
			continue;

		if (mute_)
			playQueue_.clear();

		if (!playQueue_.getFront(tmp))
		{
			tmp.erase();
			tmp.pushBack((unsigned char)0, bufferNotifySize_, true);		//静音
		}

		pDSB8_->Lock(offset, bufferNotifySize_, &buf, &buf_len, NULL, NULL, 0);
		memcpy(buf, tmp.beginRead(), tmp.readableBytes());
		pDSB8_->Unlock(buf, buf_len, NULL, 0);

		assert(buf_len == bufferNotifySize_);
		offset = (offset+buf_len)%(bufferNotifySize_*MAX_AUDIO_BUF);
	}

	pDSB8_->Stop();
	pDSB8_->Release();

	pDSN_->Release();

	for (int i = 0; i<MAX_AUDIO_BUF; i++)
		CloseHandle(ev[i]);

}
