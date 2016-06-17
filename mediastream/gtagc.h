#pragma once

#include "MediaContext.h"
#include "processor.h"

#define   MAX24bit   (1024*4*8)
#define	AD_RATIO	0.96 //最大值 与 0dB对应电平 的比值
#define 	REF_0dB	((float)(MAX24bit/AD_RATIO))

class Agc: public Processor
{
public:
	Agc(int vol=0, int peek_level=0, int  dynamic_threshold=-10, int noise_threshold=-20);
public:
	~Agc();
private:
	int    silence_counter;
	int	   silence_threshold;
	int	   active_threshold;
	int	   counter;
	int	   peak;
	float  gain;
	float  pk;
    short  sample_max;	
	bool   agcOn;
	int    volume;
public:
	int SetGtagc(int peek_level, int  dynamic_threshold, int noise_threshold);
	void DoGtagc(short *buffer, int len);
	virtual void process(MediaContext &mc);

	void OpenAgc();
	void CloseAgc();
};
