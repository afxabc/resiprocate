/*
   ITU-T G.729 Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

/* Quantize SID gain                                      */

#include <stdio.h>
#include <stdlib.h>
#include "g729typedef.h"
#include "g729basic_op.h"
#include "g729oper_32b.h"
#include "g729ld8k.h"
#include "g729vad.h"
#include "g729dtx.h"
#include "g729sid.h"
#include "g729tab_dtx.h"

/* Local function */
static Word16 Quant_Energy(
  Word32 L_x,    /* (i)  : Energy                 */
  Word16 sh,     /* (i)  : Exponent of the energy */
  Word16 *enerq  /* (o)  : quantized energy in dB */
);

/*-------------------------------------------------------------------*
 * Function  Qua_Sidgain                                             *
 *           ~~~~~~~~~~~                                             *
 *-------------------------------------------------------------------*/
void Qua_Sidgain(
  Word16 *ener,     /* (i)   array of energies                   */
  Word16 *sh_ener,  /* (i)   corresponding scaling factors       */
  Word16 nb_ener,   /* (i)   number of energies or               */
  Word16 *enerq,    /* (o)   decoded energies in dB              */
  Word16 *idx       /* (o)   SID gain quantization index         */
)
{
  Word16 i;
  Word32 L_x;
  Word16 sh1, temp;
  Word16 hi, lo;
  Word32 L_acc;
  
  if(nb_ener == 0) {
    /* Quantize energy saved for frame erasure case                */
    /* L_x = average_ener                                          */
    L_acc = g729L_deposit_l(*ener);
    L_acc = g729L_shl(L_acc, *sh_ener); /* >> if *sh_ener < 0 */
    L_Extract(L_acc, &hi, &lo);
    L_x = Mpy_32_16(hi, lo, fact729[0]);
    sh1 = 0;
  }
  else {
    
    /*
     * Compute weighted average of energies
     * ener[i] = enerR[i] x 2**sh_ener[i]
     * L_x = k[nb_ener] x SUM(i=0->nb_ener-1) enerR[i]
     * with k[nb_ener] =  fact_ener / nb_ener x L_FRAME x nbAcf
     */
    sh1 = sh_ener[0];
    for(i=1; i<nb_ener; i++) {
      if(sh_ener[i] < sh1) sh1 = sh_ener[i];
    }
    sh1 = g729add(sh1, (16-marg[nb_ener]));
    L_x = 0L;
    for(i=0; i<nb_ener; i++) {
      temp = g729sub(sh1, sh_ener[i]);
      L_acc = g729L_deposit_l(ener[i]);
      L_acc = g729L_shl(L_acc, temp);
      L_x = g729L_add(L_x, L_acc);
    }
    L_Extract(L_x, &hi, &lo);
    L_x = Mpy_32_16(hi, lo, fact729[i]);
  }
  
  *idx = Quant_Energy(L_x, sh1, enerq);
  
  return;
}


/* Local function */

static Word16 Quant_Energy(
  Word32 L_x,    /* (i)  : Energy                 */
  Word16 sh,     /* (i)  : Exponent of the energy */
  Word16 *enerq  /* (o)  : quantized energy in dB */
)
{

  Word16 exp, frac;
  Word16 e_tmp, temp, index;

  Log2(L_x, &exp, &frac);
  temp = g729sub(exp, sh);
  e_tmp = g729shl(temp, 10);
  e_tmp = g729add(e_tmp, g729mult_r(frac, 1024)); /* 2^10 x log2(L_x . 2^-sh) */
  /* log2(ener) = 10log10(ener) / K */
  /* K = 10 Log2 / Log10 */

  temp = g729sub(e_tmp, -2721);      /* -2721 -> -8dB */
  if(temp <= 0) {
    *enerq = -12;
    return(0);
  }

  temp = g729sub(e_tmp, 22111);      /* 22111 -> 65 dB */  
  if(temp > 0) {
    *enerq = 66;
    return(31);
  }

  temp = g729sub(e_tmp, 4762);       /* 4762 -> 14 dB */
  if(temp <= 0){
    e_tmp = g729add(e_tmp, 3401);
    index = g729mult(e_tmp, 24);
    if (index < 1) index = 1;
    *enerq = g729sub(g729shl(index, 2), 8);
    return(index);
  }

  e_tmp = g729sub(e_tmp, 340);
  index = g729sub(g729shr(g729mult(e_tmp, 193), 2), 1);
  if (index < 6) index = 6;
  *enerq = g729add(g729shl(index, 1), 4);
  return(index);
}



