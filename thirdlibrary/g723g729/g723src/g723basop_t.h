#ifndef BASOP_T_H
#define BASOP_T_H
#include "g723typedef.h"
#include "g723cst_lbc.h"
/*________________________________________________________
___________________
 |
                  |
 |   Constants and Globals
                  |

|_________________________________________________________
__________________|
*/

#define MAX_32 (Word32)0x7fffffffL
#define MIN_32 (Word32)0x80000000L

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000

#define L_MULT(var1, var2, v_out)       \
{                                       \
    v_out = (Word32)var1 * (Word32)var2;\
    if (v_out != (Word32)0x40000000L){   \
        v_out *= 2L;                    \
    }                                   \
    else{                              \
        g723Overflow = 1;                   \
        v_out = MAX_32;                 \
	}                                   \
}

#define L_ADD(var1, var2, v_out)       \
{                                     \
    v_out = var1 + var2;      \
    if (((var1 ^ var2) & MIN_32) == 0L) {  \
        if ((v_out ^ var1) & MIN_32) {   \
            v_out = (var1 < 0L) ? MIN_32 : MAX_32;   \
            g723Overflow = 1;   \
        }   \
    }   \
}

#define L_MAC(var3, var1, var2, v_out)       \
{                                      \
    Word32 L_produit;                  \
	L_MULT(var1, var2, L_produit);     \
	L_ADD(var3, L_produit, v_out);   \
}

#define L_SHR(var1, var2, v_out)       \
{                                      \
    if (var2 < (Word16)0) {            \
        v_out = g723L_shl(var1, (Word16)-var2); \
    }                                 \
    else {                            \
        if (var2 >= (Word16)31) {     \
            v_out = (var1 < 0L) ? -1L : 0L;  \
        }                             \
        else {                        \
            if (var1< 0L)  {          \
                v_out = ~((~var1) >> var2);   \
            }                         \
            else {                       \
                v_out = var1 >> var2;    \
            }                           \
        }                              \
    }                                  \
}

extern DECSTATDEF  DecStat  ;
extern CODCNGDEF CodCng;
extern CODSTATDEF  CodStat  ;
extern DECCNGDEF DecCng;
extern VADSTATDEF  VadStat ;

/* Global variables */
extern enum  Wmode  WrkMode;
extern enum  Crate  WrkRate;

extern int  PackedFrameSize[2];

extern   Flag    UseHp;
extern   Flag    UsePf;
extern   Flag    UseVx;
extern   Flag    UsePr;

extern   char  SignOn[];
extern Flag g723Overflow;
/*________________________________________________________
___________________
 |
                  |
 |   Operators prototypes
                  |

|_________________________________________________________
__________________|
*/

//Word16 g723add(Word16 var1, Word16 var2);     /* Short g723add,           1 */
Word16 g723sub(Word16 var1, Word16 var2);     /* Short g723sub,           1 */
Word16 g723abs_s(Word16 var1);                /* Short abs,           1 */
Word16 g723shl(Word16 var1, Word16 var2);     /* Short shift left,    1 */
Word16 g723shr(Word16 var1, Word16 var2);     /* Short shift right,   1 */
Word16 g723mult(Word16 var1, Word16 var2);    /* Short g723mult,          1 */
Word32 g723L_mult(Word16 var1, Word16 var2);  /* Long g723mult,           1 */
Word16 g723negate(Word16 var1);               /* Short negate,        1 */
Word16 g723extract_h(Word32 L_var1);          /* Extract high,        1 */
Word16 g723extract_l(Word32 L_var1);          /* Extract low,         1 */
Word16 g723round(Word32 L_var1);              /* Round,               1 */
Word32 g723L_mac(Word32 L_var3, Word16 var1, Word16 var2); /* Mac,    1 */
Word32 g723L_msu(Word32 L_var3, Word16 var1, Word16 var2); /* Msu,    1 */
Word32 g723L_macNs(Word32 L_var3, Word16 var1, Word16 var2);/*[Mac without sat, 
1*/
Word32 g723L_msuNs(Word32 L_var3, Word16 var1, Word16 var2);/* Msu without sat, 
1*/

Word32 g723L_add(Word32 L_var1, Word32 L_var2);   /* Long g723add,        2 */
Word32 g723L_sub(Word32 L_var1, Word32 L_var2);   /* Long g723sub,        2 */
Word32 g723L_add_c(Word32 L_var1, Word32 L_var2); /*Long g723add with c,  2 */
Word32 g723L_sub_c(Word32 L_var1, Word32 L_var2); /*Long g723sub with c,  2 */
Word32 g723L_negate(Word32 L_var1);               /* Long negate,     2 */
Word16 g723mult_r(Word16 var1, Word16 var2);  /* Mult with g723round,     2 */
Word32 g723L_shl(Word32 L_var1, Word16 var2); /* Long shift left,     2 */
Word32 g723L_shr(Word32 L_var1, Word16 var2); /* Long shift right,    2 */
Word16 g723shr_r(Word16 var1, Word16 var2);/* Shift right with g723round, 2 */
Word16 g723mac_r(Word32 L_var3, Word16 var1, Word16 var2);/* Mac with rounding, 
2*/
Word16 g723msu_r(Word32 L_var3, Word16 var1, Word16 var2);/* Msu with rounding, 
2*/
Word32 g723L_deposit_h(Word16 var1);       /* 16 bit var1 -> MSB,     2 */
Word32 g723L_deposit_l(Word16 var1);       /* 16 bit var1 -> LSB,     2 */

Word32 g723L_shr_r(Word32 L_var1, Word16 var2);/* Long shift right with g723round, 
 3*/
Word32 g723L_abs(Word32 L_var1);            /* Long abs,              3 */

Word32 g723L_sat(Word32 L_var1);            /* Long saturation,       4 */

Word16 g723norm_s(Word16 var1);             /* Short norm,           15 */

Word16 g723div_s(Word16 var1, Word16 var2); /* Short division,       18 */

Word16 g723norm_l(Word32 L_var1);           /* Long norm,            30 */


/*
   Additional operators
*/
Word32 L_mls( Word32, Word16 ) ;        /* Wght ?? */
Word16 div_l( Word32, Word16 ) ;
Word16 i_mult(Word16 a, Word16 b);

#endif