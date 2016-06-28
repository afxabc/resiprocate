#pragma once


//ÉùÒôÊäÈë£ºÂó¿Ë·ç
class AudioRead
{
public:
	AudioRead();
	~AudioRead();

public:
	bool start(BYTE payload, UINT rate);
	void stop();
};

