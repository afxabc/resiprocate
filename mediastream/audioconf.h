#ifndef AUDIOCONF_H
#define AUDIOCONF_H

#include "str_utils.h"
#include <vector>

class WincardWrite;

class AudioConf
{
public:
	AudioConf();
	~AudioConf();

	void addMember(WincardWrite* member);
	void remMember(WincardWrite* member);

	int getTotalMember();
	void readBufferizer(int line, char *data, int len);

private:
	std::vector<WincardWrite*> memberChain;

};

#endif