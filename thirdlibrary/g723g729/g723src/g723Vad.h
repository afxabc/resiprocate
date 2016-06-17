#ifndef VAD_H
#define VAD_H
/*
**
** File:        "vad.h"
**
** Description:     Function prototypes for "vad.c"
**  
*/

/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#include "g723typedef.h"
#include "g723basop_t.h"

//void    Init_Vad(void) ;
Flag Comp_Vad( Word16 *Dpnt);


#endif