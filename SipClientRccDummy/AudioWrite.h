#pragma once

#include "rutil\ThreadIf.hxx"
#include "buffer.h"
#include "queue.h"

#include <mmsystem.h>
#include <dsound.h>
#pragma comment(lib, "dsound.lib")
#pragma comment(lib,"dxguid.lib")

//ÉùÒô²¥·Å£ºÉù¿¨ÑïÉùÆ÷
class AudioWrite : public resip::ThreadIf
{
public:
	AudioWrite();
	~AudioWrite();

public:
	bool start(UINT rate);
	void stop();
	void inputPcm(const char* data, int len);

	bool isStart() { return !mShutdown; }
	void setMute(bool mute = true) { mute_ = mute; }

private:
	// Í¨¹ý ThreadIf ¼Ì³Ð
	virtual void thread() override;

private:
	static const int MAX_AUDIO_BUF = 4;

	WAVEFORMATEX fmtWave_;
	LPDIRECTSOUND8 lpDSound_;
	LPDIRECTSOUNDBUFFER8 pDSB8_;
	LPDIRECTSOUNDNOTIFY pDSN_;

	bool mute_;
	UINT bufferNotifySize_;
	Buffer recvBuff_;
	Queue<Buffer> playQueue_;
};



