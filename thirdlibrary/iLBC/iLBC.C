

#include "stdafx.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "iLBC_define.h"
#include "iLBC_encode.h"
#include "iLBC_decode.h"

#define ILBCNOOFWORDS_MAX   (NO_OF_BYTES_30MS/2)


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

_declspec(dllexport) short InitEncode(/* (o) Number of bytes
                                              encoded */
       iLBC_Enc_Inst_t *iLBCenc_inst,  /* (i/o) Encoder instance */
       int mode                    /* (i) frame size mode */
   )
{
	return initEncode(iLBCenc_inst,mode); 
}

_declspec(dllexport) short InitDecode( /* (o) Number of decoded
                                              samples */
       iLBC_Dec_Inst_t *iLBCdec_inst,  /* (i/o) Decoder instance */
       int mode,                       /* (i) frame size mode */
       int use_enhancer                /* (i) 1 to use enhancer
                                              0 to run without
                                                enhancer */
   )
{
  return initDecode(iLBCdec_inst,mode,use_enhancer);
}

_declspec(dllexport)  short encode(   /* (o) Number of bytes encoded */
       iLBC_Enc_Inst_t *iLBCenc_inst,
                                   /* (i/o) Encoder instance */
       short *encoded_data,    /* (o) The encoded bytes */
       short *data                 /* (i) The signal block to encode*/
   ){
       float block[BLOCKL_MAX];
       int k;

       /* convert signal to float */

       for (k=0; k<iLBCenc_inst->blockl; k++)
           block[k] = (float)data[k];

       /* do the actual encoding */

       iLBC_encode((unsigned char *)encoded_data, block, iLBCenc_inst);


       return (iLBCenc_inst->no_of_bytes);
   }


_declspec(dllexport) short decode(       /* (o) Number of decoded samples */
       iLBC_Dec_Inst_t *iLBCdec_inst,  /* (i/o) Decoder instance */
       short *decoded_data,        /* (o) Decoded signal block*/
       short *encoded_data,        /* (i) Encoded bytes */
       short mode                       /* (i) 0=PL, 1=Normal */
   ){
       int k;
       float decblock[BLOCKL_MAX], dtmp;

       /* check if mode is valid */

       if (mode<0 || mode>1) {
           printf("\nERROR - Wrong mode - 0, 1 allowed\n"); exit(3);}

       /* do actual decoding of block */

       iLBC_decode(decblock, (unsigned char *)encoded_data,
           iLBCdec_inst, mode);

       /* convert to short */
for (k=0; k<iLBCdec_inst->blockl; k++){
           dtmp=decblock[k];

           if (dtmp<MIN_SAMPLE)
               dtmp=MIN_SAMPLE;
           else if (dtmp>MAX_SAMPLE)
               dtmp=MAX_SAMPLE;
           decoded_data[k] = (short) dtmp;
       }

       return (iLBCdec_inst->blockl);
}


 


