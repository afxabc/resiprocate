/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/
extern Flag g729Overflow;
extern Flag g729Carry;

#define MAX_32 (Word32)0x7fffffffL
#define MIN_32 (Word32)0x80000000L

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000


/*___________________________________________________________________________
 |                                                                           |
 |   Operators prototypes                                                    |
 |___________________________________________________________________________|
*/

Word16 g729sature(Word32 L_var1);             /* Limit to 16 bits,    1 */
//Word16 g729add(Word16 var1, Word16 var2);     /* Short g729add,           1 */
Word16 g729sub(Word16 var1, Word16 var2);     /* Short g729sub,           1 */
Word16 g729abs_s(Word16 var1);                /* Short abs,           1 */
Word16 g729shl(Word16 var1, Word16 var2);     /* Short shift left,    1 */
Word16 g729shr(Word16 var1, Word16 var2);     /* Short shift right,   1 */
Word16 g729mult(Word16 var1, Word16 var2);    /* Short g729mult,          1 */
Word32 g729L_mult(Word16 var1, Word16 var2);  /* Long g729mult,           1 */
Word16 g729negate(Word16 var1);               /* Short negate,        1 */
Word16 g729extract_h(Word32 L_var1);          /* Extract high,        1 */
Word16 g729extract_l(Word32 L_var1);          /* Extract low,         1 */
Word16 g729round(Word32 L_var1);              /* Round,               1 */
Word32 g729L_mac(Word32 L_var3, Word16 var1, Word16 var2); /* Mac,    1 */
Word32 g729L_msu(Word32 L_var3, Word16 var1, Word16 var2); /* Msu,    1 */
Word32 g729L_macNs(Word32 L_var3, Word16 var1, Word16 var2);/* Mac without sat, 1*/
Word32 g729L_msuNs(Word32 L_var3, Word16 var1, Word16 var2);/* Msu without sat, 1*/

Word32 g729L_add(Word32 L_var1, Word32 L_var2);   /* Long g729add,        2 */
Word32 g729L_sub(Word32 L_var1, Word32 L_var2);   /* Long g729sub,        2 */
Word32 g729L_add_c(Word32 L_var1, Word32 L_var2); /*Long g729add with c,  2 */
Word32 g729L_sub_c(Word32 L_var1, Word32 L_var2); /*Long g729sub with c,  2 */
Word32 g729L_negate(Word32 L_var1);               /* Long negate,     2 */
Word16 g729mult_r(Word16 var1, Word16 var2);  /* Mult with g729round,     2 */
Word32 g729L_shl(Word32 L_var1, Word16 var2); /* Long shift left,     2 */
Word32 g729L_shr(Word32 L_var1, Word16 var2); /* Long shift right,    2 */
Word16 g729shr_r(Word16 var1, Word16 var2);/* Shift right with g729round, 2 */
Word16 g729mac_r(Word32 L_var3, Word16 var1, Word16 var2);/* Mac with rounding, 2*/
Word16 g729msu_r(Word32 L_var3, Word16 var1, Word16 var2);/* Msu with rounding, 2*/
Word32 g729L_deposit_h(Word16 var1);       /* 16 bit var1 -> MSB,     2 */
Word32 g729L_deposit_l(Word16 var1);       /* 16 bit var1 -> LSB,     2 */

Word32 g729L_shr_r(Word32 L_var1, Word16 var2);/* Long shift right with g729round,  3*/
Word32 g729L_abs(Word32 L_var1);            /* Long abs,              3 */

Word32 g729L_sat(Word32 L_var1);            /* Long saturation,       4 */

Word16 g729norm_s(Word16 var1);             /* Short norm,           15 */

Word16 g729div_s(Word16 var1, Word16 var2); /* Short division,       18 */

Word16 g729norm_l(Word32 L_var1);           /* Long norm,            30 */

