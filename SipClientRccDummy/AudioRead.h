#pragma once


//�������룺��˷�
class AudioRead
{
public:
	AudioRead();
	~AudioRead();

public:
	bool start(BYTE payload, UINT rate);
	void stop();
};

