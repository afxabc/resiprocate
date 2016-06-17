#ifndef TAME_H
#define TAME_H
/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.0
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#include "typedef.h"
#include "Cst_lbc.h"
#include "tab_lbc.h"
#include "basop_t.h"

void Update_Err(Word16 Olp, Word16 AcLg, Word16 AcGn);
Word16 Test_Err(Word16 Lag1, Word16 Lag2);

#endif