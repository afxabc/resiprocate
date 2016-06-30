#pragma once


#include "rutil\ThreadIf.hxx"
#include "buffer.h"
#include "queue.h"

#include <mmsystem.h>
#include <dsound.h>

#include <vector>

class IAudioReadCallback
{
public:
	virtual void outputPcm(char* data, int len) = 0;
};

class RTPSession;
//声音输入：麦克风
class AudioRead : public resip::ThreadIf
{
public:
	AudioRead(IAudioReadCallback* callback);
	~AudioRead();

public:
	void enumDevices();

	bool start(UINT rate, int ptime);
	void stop();

	bool isStart() { return !mShutdown; }
	void setMute(bool mute = true) { mute_ = mute; }

private:
	// 通过 ThreadIf 继承
	virtual void thread() override;

private:
	static const int MAX_AUDIO_BUF = 4;

	WAVEFORMATEX fmtWave_;
	LPDIRECTSOUNDCAPTURE8 lpDSCapture_;
	LPDIRECTSOUNDCAPTUREBUFFER8 pDSB8_;
	LPDIRECTSOUNDNOTIFY pDSN_;

	bool mute_;
	UINT bufferNotifySize_;

	IAudioReadCallback* callback_;

public:
	std::vector<GUID> devGuids_;
	std::vector<CString> devNames_;
};

