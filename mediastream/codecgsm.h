#ifndef CODECGSM_H
#define CODECGSM_H

#include "basecodec.h"
#include "mediacontext.h"
#include "gsm/gsm.h"
#include "gsm/proto.h"


class CodecGsm:public BaseCodec
{
public:
	CodecGsm();
	~CodecGsm();
	virtual void encode(queue_t *payload);
	virtual mblk_t* decode(mblk_t *inm);
private:
	MSBufferizer inData;
	gsm gsm_state;
};

extern "C" {
	gsm gsm_create P0();
	void gsm_encode P3((s, source, c), gsm s, gsm_signal * source, gsm_byte * c);
	int gsm_decode P3((s, c, target), gsm s, gsm_byte * c, gsm_signal * target);
}

#endif