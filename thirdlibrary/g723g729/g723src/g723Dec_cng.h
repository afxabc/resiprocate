#ifndef DEC_CNG_H
#define DEC_CNG_H
/*
**
** File:        "dec_cng.h"
**
** Description:     Function prototypes for "dec_cng.c"
**
*/
/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#include "g723typedef.h"
#include "g723cst_lbc.h"
#include "g723basop_t.h"
#include "g723tab_lbc.h"

//void Init_Dec_Cng(void);
void Dec_Cng(Word16 Ftyp, LINEDEF *Line, Word16 *DataExc,
                                                    Word16 *QntLpc);

#endif
