#ifndef UTIL_CNG_H
#define UTIL_CNG_H
/*
**
** File:        "util_cng.h"
**
** Description:     Function prototypes for "util_cng.c"
**
*/

/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#include "typedef.h"
#include "basop_t.h"
#include "cst_lbc.h"
#include "tab_lbc.h"

void Calc_Exc_Rand(Word16 cur_gain, Word16 *PrevExc, Word16 *DataExc,
                                      Word16 *nRandom, LINEDEF *Line);
Word16 Qua_SidGain(Word16 *Ener, Word16 *shEner, Word16 nq);
Word16 Dec_SidGain(Word16 i_gain);

#endif
