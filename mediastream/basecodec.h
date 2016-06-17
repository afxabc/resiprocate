#ifndef BASECODEC_H
#define BASECODEC_H

#include "str_utils.h"

class BaseCodec
{
public:
	BaseCodec(void){};
public:
	virtual ~BaseCodec(void){};
public:
	virtual void encode(queue_t *payload)=0;
	virtual mblk_t* decode(mblk_t *inm)=0;
};

#endif
