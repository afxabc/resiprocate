#include "audioconf.h"
#include "mediastream/wincardwrite.h"

AudioConf::AudioConf()
{
}

AudioConf::~AudioConf()
{
}

void AudioConf::addMember(WincardWrite* member)
{
	memberChain.push_back(member);
}

void AudioConf::remMember(WincardWrite* member)
{
	for (std::vector<WincardWrite*>::iterator i = memberChain.begin(); i != memberChain.end(); ++i)
	{
		if((*i)==member)
		{
			std::vector<WincardWrite*>::iterator tmp=i;
			i++;
			memberChain.erase(tmp);
			return;
		}
	}
}

int AudioConf::getTotalMember()
{
	return memberChain.size();
}

void AudioConf::readBufferizer(int line, char *data, int len)
{
	short pdata[640];
	for (std::vector<WincardWrite*>::iterator i = memberChain.begin(); i != memberChain.end(); ++i)
	{
		if((*i)->getPID() != line)
		{
			int size = ms_bufferizer_get_avail((*i)->getConfBufptr());
			if(size >= len)
			{
				ms_bufferizer_read((*i)->getConfBufptr(), (unsigned char*)pdata, len);
				for(int j=0; j<len/2; j++)
					*(((short*)data)+j) += pdata[j]/getTotalMember();
			}
		}
	}
}

