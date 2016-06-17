#ifndef COD_CNG_H
#define COD_CNG_H
/*
**
** File:        "cod_cng.h"
**
** Description:     Function prototypes for "cod_cng.c"
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

//void Init_Cod_Cng(void);
void Cod_Cng(Word16 *DataExc, Word16 *Ftyp, LINEDEF *Line, Word16 *QntLpc);
void Update_Acf(Word16 *Acfsf, Word16 *Shsf);

#endif
