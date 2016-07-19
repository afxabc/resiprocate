#ifndef TAB_LBC_H
#define TAB_LBC_H
/*
**
** File:        "tab_lbc.h"
**
** Description:  This file contains extern declarations of the tables used
**               by the SG15 LBC Coder for 6.3/5.3 kbps.
**
*/


/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#include "g723typedef.h"
#include "g723cst_lbc.h"

extern Word16   HammingWindowTable[LpcFrame];
extern Word16   BinomialWindowTable[LpcOrder];
extern Word16   BandExpTable[LpcOrder];
extern Word16   CosineTable[CosineTableSize];
extern Word16   LspDcTable[LpcOrder];
extern Word16   BandInfoTable[LspQntBands][2];
extern Word16   Band0Tb8[LspCbSize*3];
extern Word16   Band1Tb8[LspCbSize*3];
extern Word16   Band2Tb8[LspCbSize*4];
extern Word16  *BandQntTable[LspQntBands];
extern Word16   PerFiltZeroTable[LpcOrder];
extern Word16   PerFiltPoleTable[LpcOrder];
extern Word16   PostFiltZeroTable[LpcOrder];
extern Word16   PostFiltPoleTable[LpcOrder];
extern Word16   Nb_puls[4];
extern Word16   FcbkGainTable[NumOfGainLev];
extern Word32   MaxPosTable[4];
extern Word32   CombinatorialTable[MaxPulseNum][SubFrLen/Sgrid];
extern Word16 AcbkGainTable085[85*20];
extern Word16 AcbkGainTable170[170*20];
extern Word16  *AcbkGainTablePtr[2];
extern Word16   LpfConstTable[2];
extern Word16 epsi170[170];
extern Word16 gain170[170];
extern Word16 tabgain170[170];
extern Word16 tabgain85[85];
extern Word16 fact[4];
extern Word32 L_bseg[3];
extern Word16 base[3];

#endif