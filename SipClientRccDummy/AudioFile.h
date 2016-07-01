#pragma once

#include "rutil\ThreadIf.hxx"
#include "buffer.h"
#include "queue.h"

#include <mmsystem.h>

class IAudioReadCallback;
class AudioFile : public resip::ThreadIf
{
public:
	AudioFile(IAudioReadCallback* callback);
	~AudioFile();

public:
	bool start(UINT rate, int ptime);
	void stop();

	bool isStart() { return !mShutdown; }

	void signalUp()
	{
		tmSig_.on();
	}

private:
	// Í¨¹ý ThreadIf ¼Ì³Ð
	virtual void thread() override;

private:
	UINT bufferNotifySize_;
	char* pWavData_;
	DWORD szWavData_;
	int ptime_;
	Signal tmSig_;

	IAudioReadCallback* callback_;

};

