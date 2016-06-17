#ifndef CODECG729_H
#define CODECG729_H

#include "basecodec.h"
#include "mediacontext.h"
#include "g723g729/g729src/g729typedef.h"
#include "g723g729/g729src/g729ld8k.h"
#include "g723g729/g729src/g729dtx.h"
#include "g723g729/g729src/g729basic_op.h"


class CodecG729:public BaseCodec
{
public:
	CodecG729();
	~CodecG729();
	virtual void encode(queue_t *payload);
	virtual mblk_t* decode(mblk_t *inm);
private:
	MSBufferizer inData;
	MSBufferizer outData;
	//coder
	Word16  prm[PRM_SIZE+1];
	Word16  frame;
	Word16  syn[L_FRAME];
	Word16  cserial[SERIAL_SIZE];

	//decoder
	Word16  synth_buf[L_FRAME+M], *synth;
	Word16  voicing;
	Word16  Az_dec[MP1*2], *ptr_Az;
	Word16  parm[PRM_SIZE+2];
	Word16  T0_first;
	Word16  Vad;
	Word16  pst_out[L_FRAME];
	Word16  sf_voic;
	Word16  dserial[SERIAL_SIZE];
};

extern "C" {
	void Init_Pre_Process(void);
	void Init_Post_Process(void);
	void Init_Coder_ld8k(void);
	void Init_Cod_cng(void);
	void Init_Dec_cng(void);
	void Init_Decod_ld8k(void);
	void Init_Post_Filter(void);
	Word16 g729add(Word16 var1, Word16 var2);

	void Pre_Process(
	  Word16 signal[],  /* (i/o)   : input/output signal                        */
	  Word16 lg         /* (i)     : length of signal                           */
	);
	void Post_Process(
	  Word16 signal[],  /* (i/o)   : input/output signal                        */
	  Word16 lg         /* (i)     : length of signal                           */
	);
	void  prm2bits_ld8k(
	  Word16 prm[],     /* (i)     : coder parameters                           */
	  Word16 bits[]     /* (o)     : bit stream                                 */
	);
	void  bits2prm_ld8k(
	  Word16 bits[],    /* (i)     : bit stream                                 */
	  Word16 prm[]      /* (o)     : coder parameters                           */
	);
	void Coder_ld8k(
	  Word16 ana[],     /* (o)     : analysis parameters                       */
	  Word16 synth[],   /* (o)     : local synthesis                           */
	  Word16 frame,     /* (i)     : frame counter                             */
	  Word16 vad_enable /* (i)     : VAD enable flag                           */
	);
	void Decod_ld8k(
	  Word16 parm[],   /* (i)     : vector of synthesis parameters
									  parm[0] = bad frame indicator (bfi)      */
	  Word16 voicing,  /* (i)     : voicing decision from previous frame       */
	  Word16 synth[],  /* (o)     : synthesized speech                         */
	  Word16 A_t[],    /* (o)     : decoded LP filter for 2 subframes          */
	  Word16 *T0_first,/* (o)     : decoded pitch lag in first subframe        */
	  Word16 *Vad      /* (o)     : frame type                                 */
	);
	void Copy(
	  Word16 x[],       /* (i)     : input vector                               */
	  Word16 y[],       /* (o)     : output vector                              */
	  Word16 L          /* (i)     : vector length                              */
	);
	void Post(
	  Word16 t0,        /* (i) : 1st subframe delay given by coder              */
	  Word16 *signal_ptr, /* (i) : input signal (pointer to current subframe    */
	  Word16 *coeff,    /* (i) : LPC coefficients for current subframe          */
	  Word16 *sig_out,  /* (o) : postfiltered output                            */
	  Word16 *vo,       /* (o) : voicing decision 0 = uv,  > 0 delay            */
	  Word16 Vad        /* (i) : frame type                                     */
	);
	Word16 Check_Parity_Pitch( /* (o) : 0 = no error, 1= error                  */
	  Word16 pitch_index, /* (i)   : index of parameter                         */
	  Word16 parity       /* (i)   : parity bit                                 */
	);
	Word16 * GetNewSpeech();
}

#endif