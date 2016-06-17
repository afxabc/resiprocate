/*
   ITU-T G.729 Annex B     ANSI-C Source Code
   Version 1.3    Last modified: August 1997

   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

/* Computation of Comfort Noise excitation             */

#include <stdio.h>
#include <stdlib.h>

#include "g729typedef.h"
#include "g729ld8k.h"
#include "g729dtx.h"
#include "g729basic_op.h"
#include "g729oper_32b.h"


/* Local functions */
static Word16 Gauss(Word16 *seed);
static Word16 Sqrt( Word32 Num);

/*-----------------------------------------------------------*
 * procedure Calc_exc_rand                                   *
 *           ~~~~~~~~~~~~~                                   *
 *   Computes comfort noise excitation                       *
 *   for SID and not-transmitted frames                      *
 *-----------------------------------------------------------*/
void Calc_exc_rand(
  Word16 cur_gain,      /* (i)   :   target sample gain                 */
  Word16 *exc,          /* (i/o) :   excitation array                   */
  Word16 *seed,         /* (i)   :   current Vad decision               */
  Flag flag_cod         /* (i)   :   encoder/decoder flag               */
)
{
  Word16 i, j, i_subfr;
  Word16 temp1, temp2;
  Word16 pos[4];
  Word16 sign[4];
  Word16 t0, frac;
  Word16 *cur_exc;
  Word16 g, Gp, Gp2;
  Word16 excg[L_SUBFR], excs[L_SUBFR];
  Word32 L_acc, L_ener, L_k;
  Word16 max, hi, lo, inter_exc;
  Word16 sh;
  Word16 x1, x2;
  
  if(cur_gain == 0) {

    for(i=0; i<L_FRAME; i++) {
      exc[i] = 0;
    }
    Gp = 0;
    t0 = g729add(L_SUBFR,1);
    for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR) {
      if(flag_cod != FLAG_DEC) update_exc_err(Gp, t0);
    }

    return;
  }

  
  
  /* Loop on subframes */
  
  cur_exc = exc;
  
  for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR) {

    /* generate random adaptive codebook & fixed codebook parameters */
    /*****************************************************************/
    temp1 = Random(seed);
    frac = g729sub((temp1 & (Word16)0x0003), 1);
    if(g729sub(frac, 2) == 0) frac = 0;
    temp1 = g729shr(temp1, 2);
    t0 = g729add((temp1 & (Word16)0x003F), 40);
    temp1 = g729shr(temp1, 6);
    temp2 = temp1 & (Word16)0x0007;
    pos[0] = g729add(g729shl(temp2, 2), temp2); /* 5 * temp2 */
    temp1 = g729shr(temp1, 3);
    sign[0] = temp1 & (Word16)0x0001;
    temp1 = g729shr(temp1, 1);
    temp2 = temp1 & (Word16)0x0007;
    temp2 = g729add(g729shl(temp2, 2), temp2);
    pos[1] = g729add(temp2, 1);     /* 5 * x + 1 */
    temp1 = g729shr(temp1, 3);
    sign[1] = temp1 & (Word16)0x0001;
    temp1 = Random(seed);
    temp2 = temp1 & (Word16)0x0007;
    temp2 = g729add(g729shl(temp2, 2), temp2);
    pos[2] = g729add(temp2, 2);     /* 5 * x + 2 */
    temp1 = g729shr(temp1, 3);
    sign[2] = temp1 & (Word16)0x0001;
    temp1 = g729shr(temp1, 1);
    temp2 = temp1 & (Word16)0x000F;
    pos[3] = g729add((temp2 & (Word16)1), 3); /* j+3*/
    temp2 = (g729shr(temp2, 1)) & (Word16)7;
    temp2 = g729add(g729shl(temp2, 2), temp2); /* 5i */
    pos[3] = g729add(pos[3], temp2);
    temp1 = g729shr(temp1, 4);
    sign[3] = temp1 & (Word16)0x0001;
    Gp = Random(seed) & (Word16)0x1FFF; /* < 0.5 Q14 */
    Gp2 = g729shl(Gp, 1);           /* Q15 */


    /* Generate gaussian excitation */
    /********************************/
    L_acc = 0L;
    for(i=0; i<L_SUBFR; i++) {
      temp1 = Gauss(seed);
      L_acc = g729L_mac(L_acc, temp1, temp1);
      excg[i] = temp1;
    }

/*
    Compute fact = alpha x cur_gain * sqrt(L_SUBFR / Eg)
    with Eg = SUM(i=0->39) excg[i]^2
    and alpha = 0.5
    alpha x sqrt(L_SUBFR)/2 = 1 + FRAC1
*/
    L_acc = Inv_sqrt(g729L_shr(L_acc,1));  /* Q30 */
    L_Extract(L_acc, &hi, &lo);
    /* cur_gain = cur_gainR << 3 */
    temp1 = g729mult_r(cur_gain, FRAC1);
    temp1 = g729add(cur_gain, temp1);
    /* <=> alpha x cur_gainR x 2^2 x sqrt(L_SUBFR) */

    L_acc = Mpy_32_16(hi, lo, temp1);   /* fact << 17 */
    sh = g729norm_l(L_acc);
    temp1 = g729extract_h(g729L_shl(L_acc, sh));  /* fact << (sh+1) */

    sh = g729sub(sh, 14);
    for(i=0; i<L_SUBFR; i++) {
      temp2 = g729mult_r(excg[i], temp1);
      temp2 = g729shr_r(temp2, sh);   /* g729shl if sh < 0 */
      excg[i] = temp2;
    }

    /* generate random  adaptive excitation */
    /****************************************/
    Pred_lt_3(cur_exc, t0, frac, L_SUBFR);


    /* compute adaptive + gaussian exc -> cur_exc */
    /**********************************************/
    max = 0;
    for(i=0; i<L_SUBFR; i++) {
      temp1 = g729mult_r(cur_exc[i], Gp2);
      temp1 = g729add(temp1, excg[i]); /* may overflow */
      cur_exc[i] = temp1;
      temp1 = g729abs_s(temp1);
      if(g729sub(temp1,max) > 0) max = temp1;
    }

    /* rescale cur_exc -> excs */
    if(max == 0) sh = 0;
    else {
      sh = g729sub(3, g729norm_s(max));
      if(sh <= 0) sh = 0;
    }
    for(i=0; i<L_SUBFR; i++) {
      excs[i] = g729shr(cur_exc[i], sh);
    }

    /* Compute fixed code gain */
    /***************************/

    /**********************************************************/
    /*** Solve EQ(X) = 4 X**2 + 2 b X + c                     */
    /**********************************************************/

    L_ener = 0L;
    for(i=0; i<L_SUBFR; i++) {
      L_ener = g729L_mac(L_ener, excs[i], excs[i]);
    } /* ener x 2^(-2sh + 1) */

    /* inter_exc = b >> sh */
    inter_exc = 0;
    for(i=0; i<4; i++) {
      j = pos[i];
      if(sign[i] == 0) {
        inter_exc = g729sub(inter_exc, excs[j]);
      }
      else {
        inter_exc = g729add(inter_exc, excs[j]);
      }
    }

    /* Compute k = cur_gainR x cur_gainR x L_SUBFR */
    L_acc = g729L_mult(cur_gain, L_SUBFR);
    L_acc = g729L_shr(L_acc, 6);
    temp1 = g729extract_l(L_acc);   /* cur_gainR x L_SUBFR x 2^(-2) */
    L_k   = g729L_mult(cur_gain, temp1); /* k << 2 */
    temp1 = g729add(1, g729shl(sh,1));
    L_acc = g729L_shr(L_k, temp1);  /* k x 2^(-2sh+1) */

    /* Compute delta = b^2 - 4 c */
    L_acc = g729L_sub(L_acc, L_ener); /* - 4 c x 2^(-2sh-1) */
    inter_exc = g729shr(inter_exc, 1);
    L_acc = g729L_mac(L_acc, inter_exc, inter_exc); /* 2^(-2sh-1) */
    sh = g729add(sh, 1);
    /* inter_exc = b x 2^(-sh) */
    /* L_acc = delta x 2^(-2sh+1) */

    if(L_acc < 0) {

      /* adaptive excitation = 0 */
      Copy(excg, cur_exc, L_SUBFR);
      temp1 = g729abs_s(excg[(int)pos[0]]) | g729abs_s(excg[(int)pos[1]]);
      temp2 = g729abs_s(excg[(int)pos[2]]) | g729abs_s(excg[(int)pos[3]]);
      temp1 = temp1 | temp2;
      sh = ((temp1 & (Word16)0x4000) == 0) ? (Word16)1 : (Word16)2;
      inter_exc = 0;
      for(i=0; i<4; i++) {
        temp1 = g729shr(excg[(int)pos[i]], sh);
        if(sign[i] == 0) {
          inter_exc = g729sub(inter_exc, temp1);
        }
        else {
          inter_exc = g729add(inter_exc, temp1);
        }
      } /* inter_exc = b >> sh */
      L_Extract(L_k, &hi, &lo);
      L_acc = Mpy_32_16(hi, lo, K0); /* k x (1- alpha^2) << 2 */
      temp1 = g729sub(g729shl(sh, 1), 1); /* temp1 > 0 */
      L_acc = g729L_shr(L_acc, temp1); /* 4k x (1 - alpha^2) << (-2sh+1) */
      L_acc = g729L_mac(L_acc, inter_exc, inter_exc); /* delta << (-2sh+1) */
      Gp = 0;
    }

    temp2 = Sqrt(L_acc);        /* >> sh */
    x1 = g729sub(temp2, inter_exc);
    x2 = g729negate(g729add(inter_exc, temp2)); /* x 2^(-sh+2) */
    if(g729sub(g729abs_s(x2),g729abs_s(x1)) < 0) x1 = x2;
    temp1 = g729sub(2, sh);
    g = g729shr_r(x1, temp1);       /* g729shl if temp1 < 0 */
    if(g >= 0) {
      if(g729sub(g, G_MAX) > 0) g = G_MAX;
    }
    else {
      if(g729add(g, G_MAX) < 0) g = g729negate(G_MAX);
    }

    /* Update cur_exc with ACELP excitation */
    for(i=0; i<4; i++) {
      j = pos[i];
      if(sign[i] != 0) {
        cur_exc[j] = g729add(cur_exc[j], g);
      }
      else {
        cur_exc[j] = g729sub(cur_exc[j], g);
      }
    }

    if(flag_cod != FLAG_DEC) update_exc_err(Gp, t0);

    cur_exc += L_SUBFR;


  } /* end of loop on subframes */
  
  return;
}

/*-----------------------------------------------------------*
 *         Local procedures                                  *
 *         ~~~~~~~~~~~~~~~~                                  *
 *-----------------------------------------------------------*/

/* Gaussian generation */
/***********************/
static Word16 Gauss(Word16 *seed)
{

/****  Xi = uniform v.a. in [-32768, 32767]       ****/
/****  Z = SUM(i=1->12) Xi / 2 x 32768 is N(0,1)  ****/
/****  output : Z x 512 < 2^12                    ****/

  Word16 i;
  Word16 temp;
  Word32 L_acc;
  
  L_acc = 0L;
  for(i=0; i<12; i++) {
    L_acc = g729L_add(L_acc, g729L_deposit_l(Random(seed)));
  }
  L_acc = g729L_shr(L_acc, 7);
  temp = g729extract_l(L_acc);
  return(temp);
}

/* Square root function : returns sqrt(Num/2) */
/**********************************************/
static Word16   Sqrt( Word32 Num )
{
  Word16   i  ;
  
  Word16   Rez = (Word16) 0 ;
  Word16   Exp = (Word16) 0x4000 ;
  
  Word32   Acc, L_temp;
  
  for ( i = 0 ; i < 14 ; i ++ ) {
    Acc = g729L_mult(g729add(Rez, Exp), g729add(Rez, Exp) );
    L_temp = g729L_sub(Num, Acc);
    if(L_temp >= 0L) Rez = g729add( Rez, Exp);
    Exp = g729shr( Exp, (Word16) 1 ) ;
  }
  return Rez ;
}





 
    





