#include "stdafx.h"
#include "AudioFile.h"

#include "AudioRead.h"
#include "resource.h"

AudioFile::AudioFile(IAudioReadCallback * callback)
	: callback_(callback)
	, pWavData_(NULL)
	, szWavData_(0)
{
}

AudioFile::~AudioFile()
{
	stop();
}

bool AudioFile::start(UINT rate, int ptime)
{
	stop();

	ptime_ = ptime;
	bufferNotifySize_ = rate * 2 *ptime / 1000;

	HINSTANCE hInstance = AfxFindResourceHandle(MAKEINTRESOURCE(IDR_WAVE1), RT_RCDATA);
	HRSRC hRes = FindResourceEx(hInstance, _T("WAVE"), MAKEINTRESOURCE(IDR_WAVE1), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
	HGLOBAL hResData = LoadResource(hInstance, hRes);

	szWavData_ = SizeofResource(NULL, hRes);
	pWavData_ = (char*)LockResource(hResData);
	if (pWavData_ == NULL || szWavData_ <= 0)
	{
		TRACE("LockResource failed !!\n");
		return false;
	}

	if (pWavData_[0] == 'R' &&pWavData_[1] == 'I' &&pWavData_[2] == 'F' &&pWavData_[3] == 'F')
	{
		pWavData_ += 44;
		szWavData_ -= 44;
	}
	

	this->run();
	return true;
}

void AudioFile::stop()
{
	if (mShutdown)
		return;

	mShutdown = true;
	this->shutdown();
	this->join();

	pWavData_ = NULL;
	szWavData_ = 0;
}

static void CALLBACK TimerProc(UINT uiID, UINT uiMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	AudioFile* pThis = (AudioFile*)dwUser;
	if (pThis != NULL)
		pThis->signalUp();
}

void AudioFile::thread()
{
	MMRESULT mRes = ::timeSetEvent(ptime_, 0, &TimerProc, (DWORD)this, TIME_PERIODIC);

	mShutdown = false;
	szWavData_ = szWavData_ / bufferNotifySize_*bufferNotifySize_;
	int offset = 0;
	int span = bufferNotifySize_;
	while (!mShutdown && szWavData_ >= bufferNotifySize_)
	{
		tmSig_.wait();
		if (mShutdown)
			break;

		if (offset < 0)
		{
			offset++;
			continue;
		}

		if (callback_)
			callback_->outputPcm(pWavData_ + offset, span);
		offset += span;

		if (offset >= szWavData_)
			offset = -(1000/ ptime_);
	}

	mRes = ::timeKillEvent(mRes);
}
