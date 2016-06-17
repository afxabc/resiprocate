/*
   ITU-T G.729 Speech Coder ANSI-C Source Code
   Version 3.3    Last modified: December 26, 1995

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

#include <stdio.h>
#include "g729typedef.h"
#include "g729basic_op.h"
#include "g729ld8k.h"


void Lsp_get_quant(
  Word16 lspcb1[][M],      /* (i) Q13 : first stage LSP codebook      */
  Word16 lspcb2[][M],      /* (i) Q13 : Second stage LSP codebook     */
  Word16 code0,               /* (i)     : selected code of first stage  */
  Word16 code1,               /* (i)     : selected code of second stage */
  Word16 code2,               /* (i)     : selected code of second stage */
  Word16 fg[][M],          /* (i) Q15 : MA prediction coef.           */
  Word16 freq_prev[][M],   /* (i) Q13 : previous LSP vector           */
  Word16 lspq[],              /* (o) Q13 : quantized LSP parameters      */
  Word16 fg_sum[]             /* (i) Q15 : present MA prediction coef.   */
)
{
  Word16 j;
  Word16 buf[M];           /* Q13 */


  for ( j = 0 ; j < NC ; j++ )
    buf[j] = g729add( lspcb1[code0][j], lspcb2[code1][j] );

  for ( j = NC ; j < M ; j++ )
    buf[j] = g729add( lspcb1[code0][j], lspcb2[code2][j] );

  Lsp_expand_1_2(buf, GAP1);
  Lsp_expand_1_2(buf, GAP2);

  Lsp_prev_compose(buf, lspq, fg, freq_prev, fg_sum);

  Lsp_prev_update(buf, freq_prev);

  Lsp_stability( lspq );

  return;
}


void Lsp_expand_1(
  Word16 buf[],        /* (i/o) Q13 : LSP vectors */
  Word16 gap           /* (i)   Q13 : gap         */
)
{
  Word16 j, tmp;
  Word16 diff;        /* Q13 */

  for ( j = 1 ; j < NC ; j++ ) {
    diff = g729sub( buf[j-1], buf[j] );
    tmp = g729shr( g729add( diff, gap), 1 );

    if ( tmp >  0 ) {
      buf[j-1] = g729sub( buf[j-1], tmp );
      buf[j]   = g729add( buf[j], tmp );
    }
  }
  return;
}


void Lsp_expand_2(
  Word16 buf[],       /* (i/o) Q13 : LSP vectors */
  Word16 gap          /* (i)   Q13 : gap         */
)
{
  Word16 j, tmp;
  Word16 diff;        /* Q13 */

  for ( j = NC ; j < M ; j++ ) {
    diff = g729sub( buf[j-1], buf[j] );
    tmp = g729shr( g729add( diff, gap), 1 );

    if ( tmp > 0 ) {
      buf[j-1] = g729sub( buf[j-1], tmp );
      buf[j]   = g729add( buf[j], tmp );
    }
  }
  return;
}


void Lsp_expand_1_2(
  Word16 buf[],       /* (i/o) Q13 : LSP vectors */
  Word16 gap          /* (i)   Q13 : gap         */
)
{
  Word16 j, tmp;
  Word16 diff;        /* Q13 */

  for ( j = 1 ; j < M ; j++ ) {
    diff = g729sub( buf[j-1], buf[j] );
    tmp = g729shr( g729add( diff, gap), 1 );

    if ( tmp > 0 ) {
      buf[j-1] = g729sub( buf[j-1], tmp );
      buf[j]   = g729add( buf[j], tmp );
    }
  }
  return;
}


/*
  Functions which use previous LSP parameter (freq_prev).
*/


/*
  compose LSP parameter from elementary LSP with previous LSP.
*/
void Lsp_prev_compose(
  Word16 lsp_ele[],             /* (i) Q13 : LSP vectors                 */
  Word16 lsp[],                 /* (o) Q13 : quantized LSP parameters    */
  Word16 fg[][M],            /* (i) Q15 : MA prediction coef.         */
  Word16 freq_prev[][M],     /* (i) Q13 : previous LSP vector         */
  Word16 fg_sum[]               /* (i) Q15 : present MA prediction coef. */
)
{
  Word16 j, k;
  Word32 L_acc;                 /* Q29 */

  for ( j = 0 ; j < M ; j++ ) {
    L_acc = g729L_mult( lsp_ele[j], fg_sum[j] );
    for ( k = 0 ; k < MA_NP ; k++ )
      L_acc = g729L_mac( L_acc, freq_prev[k][j], fg[k][j] );

    lsp[j] = g729extract_h(L_acc);
  }
  return;
}


/*
  extract elementary LSP from composed LSP with previous LSP
*/
void Lsp_prev_extract(
  Word16 lsp[M],                 /* (i) Q13 : unquantized LSP parameters  */
  Word16 lsp_ele[M],             /* (o) Q13 : target vector               */
  Word16 fg[MA_NP][M],           /* (i) Q15 : MA prediction coef.         */
  Word16 freq_prev[MA_NP][M],    /* (i) Q13 : previous LSP vector         */
  Word16 fg_sum_inv[M]           /* (i) Q12 : inverse previous LSP vector */
)
{
  Word16 j, k;
  Word32 L_temp;                    /* Q19 */
  Word16 temp;                      /* Q13 */


  for ( j = 0 ; j < M ; j++ ) {
    L_temp = g729L_deposit_h(lsp[j]);
    for ( k = 0 ; k < MA_NP ; k++ )
      L_temp = g729L_msu( L_temp, freq_prev[k][j], fg[k][j] );

    temp = g729extract_h(L_temp);
    L_temp = g729L_mult( temp, fg_sum_inv[j] );
    lsp_ele[j] = g729extract_h( g729L_shl( L_temp, 3 ) );

  }
  return;
}


/*
  update previous LSP parameter
*/
void Lsp_prev_update(
  Word16 lsp_ele[M],             /* (i)   Q13 : LSP vectors           */
  Word16 freq_prev[MA_NP][M]     /* (i/o) Q13 : previous LSP vectors  */
)
{
  Word16 k;

  for ( k = MA_NP-1 ; k > 0 ; k-- )
    Copy(freq_prev[k-1], freq_prev[k], M);

  Copy(lsp_ele, freq_prev[0], M);
  return;
}

void Lsp_stability(
  Word16 buf[]       /* (i/o) Q13 : quantized LSP parameters      */
)
{
  Word16 j;
  Word16 tmp;
  Word32 L_diff;
  Word32 L_acc, L_accb;

  for(j=0; j<M-1; j++) {
    L_acc = g729L_deposit_l( buf[j+1] );
    L_accb = g729L_deposit_l( buf[j] );
    L_diff = g729L_sub( L_acc, L_accb );

    if( L_diff < 0L ) {
      /* exchange buf[j]<->buf[j+1] */
      tmp      = buf[j+1];
      buf[j+1] = buf[j];
      buf[j]   = tmp;
    }
  }


  if( g729sub(buf[0], L_LIMIT) <0 ) {
    buf[0] = L_LIMIT;
    printf("lsp_stability warning Low \n");
  }
  for(j=0; j<M-1; j++) {
    L_acc = g729L_deposit_l( buf[j+1] );
    L_accb = g729L_deposit_l( buf[j] );
    L_diff = g729L_sub( L_acc, L_accb );

    if( g729L_sub(L_diff, GAP3)<0L ) {
      buf[j+1] = g729add( buf[j], GAP3 );
    }
  }

  if( g729sub(buf[M-1],M_LIMIT)>0 ) {
    buf[M-1] = M_LIMIT;
    printf("lsp_stability warning High \n");
  }
  return;
}
