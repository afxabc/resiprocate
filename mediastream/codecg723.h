#ifndef CODECG723_H
#define CODECG723_H

#include "basecodec.h"
#include "mediacontext.h"
#include "g723g729/g723src/g723Typedef.h"
#include "g723g729/g723src/g723Cst_lbc.h"
#include "g723g729/g723src/g723Basop_t.h"

class CodecG723:public BaseCodec
{
public:
	CodecG723();
	~CodecG723();
	virtual void encode(queue_t *payload);
	virtual mblk_t* decode(mblk_t *inm);
private:
	MSBufferizer inData;
	MSBufferizer outData;
	//coder
};

extern "C" {
	void    Init_Coder( void);
	void    Init_Decod(void);
	void    Init_Vad(void) ;
	void    Init_Cod_Cng(void);
	void    Init_Dec_Cng(void);
	Flag    Coder( Word16 *DataBuff, char *Vout );
	Flag    Decod( Word16 *DataBuff, char *Vinp, Word16 Crc );
	void    reset_max_time(void);
}
#endif