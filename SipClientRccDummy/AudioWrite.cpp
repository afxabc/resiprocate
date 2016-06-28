#include "stdafx.h"
#include "AudioWrite.h"


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

bool AudioWrite::start(UINT rate)
{
	HRESULT hr = lpDSound_->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
	if (FAILED(hr))
	{
		TRACE("SetCooperativeLevel failed %X\n", hr);
		return false;
	}
	/*
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.cbSize = 0;//PCM��ʽʱ������/
	wfx.nAvgBytesPerSec = 16000;//���ݴ������ʣ��ֽ�/��/
	wfx.nBlockAlign = 2;//ÿ���������ֽ���/
	wfx.nChannels = 1;//ͨ����/
	wfx.nSamplesPerSec = 8000;//������/
	wfx.wBitsPerSample = 16;//ÿ��������bit��/
	*/
	fmtWave_.wFormatTag = WAVE_FORMAT_PCM;
	fmtWave_.cbSize = sizeof(fmtWave_);
	fmtWave_.nAvgBytesPerSec = rate*2;
	fmtWave_.nBlockAlign = 2;
	fmtWave_.nChannels = 1;
	fmtWave_.nSamplesPerSec = rate;
	fmtWave_.wBitsPerSample = 16;

	int delay = 100; //ms
	bufferNotifySize_ = fmtWave_.nAvgBytesPerSec*delay / 1000;

	//������������������
	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFX | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes = bufferNotifySize_*MAX_AUDIO_BUF;	//((alaw_)?playSpan_*2:playSpan_); 
	dsbd.lpwfxFormat = &fmtWave_;

	pDSB8_ = NULL;
	pDSN_ = NULL;
	LPDIRECTSOUNDBUFFER pDSB = NULL;

	//��ȡ�ӿ�
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
		
	recvBuff_.pushBack(data, len, true);
	if (recvBuff_.readableBytes() < bufferNotifySize_)
		return;

	Buffer tmp;
	BYTE* buf = (BYTE*)recvBuff_.beginRead();
	tmp.pushBack((char*)buf, bufferNotifySize_);
	playQueue_.putBack(tmp);

	recvBuff_.eraseFront(bufferNotifySize_);
}

void AudioWrite::thread()
{
	int delay = 100; //ms
	
	//��������

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
		if (mute_)
			playQueue_.clear();

		if (!playQueue_.getFront(tmp))
		{
			tmp.erase();
			tmp.pushBack(0x80, bufferNotifySize_, true);		//����
		}

		if ((obj >= WAIT_OBJECT_0) && (obj < WAIT_OBJECT_0 + MAX_AUDIO_BUF))
		{
			pDSB8_->Lock(offset, bufferNotifySize_, &buf, &buf_len, NULL, NULL, 0);
			memcpy(buf, tmp.beginRead(), tmp.readableBytes());
			pDSB8_->Unlock(buf, buf_len, NULL, 0);

			offset += buf_len;
			offset %= (bufferNotifySize_*MAX_AUDIO_BUF);
		}
		obj = WaitForMultipleObjects(MAX_AUDIO_BUF, ev, FALSE, INFINITE);
	}

	pDSB8_->Stop();
	pDSB8_->Release();

	pDSN_->Release();

	for (int i = 0; i<MAX_AUDIO_BUF; i++)
		CloseHandle(ev[i]);
}
