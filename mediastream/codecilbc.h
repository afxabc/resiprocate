#ifndef CODECILBC_H
#define CODECILBC_H

#include "basecodec.h"
#include "mediacontext.h"
#include "iLBC/iLBC_define.h"


class CodeciLBC:public BaseCodec
{
public:
	CodeciLBC();
	~CodeciLBC();
	virtual void encode(queue_t *payload);
	virtual mblk_t* decode(mblk_t *inm);
private:
	MSBufferizer inData;
	MSBufferizer outData;
	iLBC_Enc_Inst_t Enc_Inst;
	iLBC_Dec_Inst_t Dec_Inst;
};

extern "C" {
	short initEncode(/* (o) Number of bytes
                                              encoded */
		iLBC_Enc_Inst_t *iLBCenc_inst,  /* (i/o) Encoder instance */
		int mode                    /* (i) frame size mode */
		);

	void iLBC_encode(

		unsigned char *bytes,           /* (o) encoded data bits iLBC */
		float *block,                   /* (o) speech vector to
                                              encode */
		iLBC_Enc_Inst_t *iLBCenc_inst   /* (i/o) the general encoder
                                              state */
		);
	short initDecode( /* (o) Number of decoded
                                              samples */
		iLBC_Dec_Inst_t *iLBCdec_inst,  /* (i/o) Decoder instance */
		int mode,                       /* (i) frame size mode */
		int use_enhancer                /* (i) 1 to use enhancer
                                              0 to run without
                                                enhancer */
		);

	void iLBC_decode(
		float *decblock,            /* (o) decoded signal block */
		unsigned char *bytes,           /* (i) encoded signal bits */
		iLBC_Dec_Inst_t *iLBCdec_inst,  /* (i/o) the decoder state
                                                structure */
		int mode                    /* (i) 0: bad packet, PLC,
                                              1: normal */
		);
}

#endif