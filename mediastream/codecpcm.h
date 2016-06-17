#ifndef CODECPCM_H
#define CODECPCM_H

#include "basecodec.h"
#include "mediacontext.h"
#include "g711common.h"

class CodecPcm:public BaseCodec
{
public:
	CodecPcm(unsigned char pt=0);
	~CodecPcm();
	virtual void encode(queue_t *payload);
	virtual mblk_t* decode(mblk_t *inm);
private:
	unsigned char type;
	MSBufferizer inData;
};

#endif