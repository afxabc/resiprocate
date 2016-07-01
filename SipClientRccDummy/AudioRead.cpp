#include "stdafx.h"
#include "AudioRead.h"


AudioRead::AudioRead(IAudioReadCallback* callback)
	: lpDSCapture_(NULL)
	, pDSB8_(NULL)
	, pDSN_(NULL)
	, mute_(false)
	, callback_(callback)
{
	HRESULT hr = DirectSoundCaptureCreate8(NULL, &lpDSCapture_, NULL);
	mShutdown = true;
}


AudioRead::~AudioRead()
{
	stop();
	lpDSCapture_->Release();
}

BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext)
{
	AudioRead* pThis = (AudioRead*)lpContext;
	if (pThis == NULL || lpGUID == NULL)
		return FALSE;

	pThis->devNames_.push_back(lpszDesc);
	pThis->devGuids_.push_back(*lpGUID);

	return TRUE;
}

void AudioRead::enumDevices()
{
	devGuids_.clear();
	devNames_.clear();
	HRESULT hr = DirectSoundCaptureEnumerate((LPDSENUMCALLBACK)DSEnumProc, (VOID*)this);
}

bool AudioRead::start(UINT rate, int ptime)
{
	stop();

	fmtWave_.wFormatTag = WAVE_FORMAT_PCM;
	fmtWave_.cbSize = sizeof(fmtWave_);
	fmtWave_.nAvgBytesPerSec = rate * 2;
	fmtWave_.nBlockAlign = 2;
	fmtWave_.nChannels = 1;
	fmtWave_.nSamplesPerSec = rate;
	fmtWave_.wBitsPerSample = 16;

	bufferNotifySize_ = fmtWave_.nAvgBytesPerSec*ptime / 1000;

	//创建辅助缓冲区对象
	DSCBUFFERDESC dscbd;
	dscbd.dwSize = sizeof(DSCBUFFERDESC);
	dscbd.dwFlags = DSCBCAPS_CTRLFX;
	dscbd.dwBufferBytes = bufferNotifySize_*MAX_AUDIO_BUF;// fmtWave_.nAvgBytesPerSec;
	dscbd.dwReserved = 0;
	dscbd.lpwfxFormat = &fmtWave_; //设置录音用的wave格式
	dscbd.dwFXCount = 0;
	dscbd.lpDSCFXDesc = NULL;

	pDSB8_ = NULL;
	pDSN_ = NULL;
	LPDIRECTSOUNDCAPTUREBUFFER pDSB = NULL;

	//获取接口
	bool res = false;
	HRESULT hr;
	do
	{
		if (FAILED(hr = lpDSCapture_->CreateCaptureBuffer(&dscbd, &pDSB, NULL)))
		{
			TRACE("CreateCaptureBuffer failed %X \n", hr);
			break;
		}
		if (FAILED(hr = pDSB->QueryInterface(IID_IDirectSoundCaptureBuffer8, (LPVOID*)&pDSB8_)))
		{
			TRACE("QueryInterface IID_IDirectSoundCaptureBuffer8 failed %X\n", hr);
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

void AudioRead::stop()
{
	if (mShutdown)
		return;

	mShutdown = true;
	this->shutdown();
	this->join();
}

void AudioRead::thread()
{
	//设置提醒

	DSBPOSITIONNOTIFY DSBPositionNotify[MAX_AUDIO_BUF];
	HANDLE ev[MAX_AUDIO_BUF];
	for (int i = 0; i<MAX_AUDIO_BUF; i++)
	{
		DSBPositionNotify[i].dwOffset = (i+1)*bufferNotifySize_-1;
		ev[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		DSBPositionNotify[i].hEventNotify = ev[i];
	}
	pDSN_->SetNotificationPositions(MAX_AUDIO_BUF, DSBPositionNotify);

	HRESULT hr = pDSB8_->Start(DSCBSTART_LOOPING);

	LPVOID buf = NULL;
	DWORD  buf_len = 0;
	DWORD obj = WAIT_OBJECT_0;
	DWORD offset = 0;

	mute_ = false;
	mShutdown = false;

	while (!mShutdown)
	{
		obj = WaitForMultipleObjects(MAX_AUDIO_BUF, ev, FALSE, INFINITE);

		if (mute_)
			continue;

		if ((obj < WAIT_OBJECT_0) || (obj >= WAIT_OBJECT_0 + MAX_AUDIO_BUF))
			continue;

///		TRACE("DSBPositionNotify[%d].dwOffset=%d\n", id, DSBPositionNotify[id].dwOffset);
//		TRACE("dwCapturePos=%d, dwReadPos=%d\n", dwCapturePos, dwReadPos);

		pDSB8_->Lock(offset, bufferNotifySize_, &buf, &buf_len, NULL, NULL, 0);
		if (callback_)
			callback_->outputPcm((char*)buf, buf_len);
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
