#ifndef EXC_LBC_H
#define EXC_LBC_H
/*
**
** File:        "lbccodec.h"
**
** Description:     Function prototypes and external declarations
**          for "lbccodec.c"
**
*/

/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/


long  Process_Files( FILE **Ifp, FILE **Ofp, FILE **Fep, FILE **Ratp,
                                        int Argc, char *Argv[] );

#endif
