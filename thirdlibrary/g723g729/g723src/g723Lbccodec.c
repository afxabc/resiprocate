/*
**
** File:        "lbccodec.c"
**
** Description:     Top-level source code for G.723 dual-rate codec
**
** Functions:       main
**                  Process_files()
**
**
*/


/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "typedef.h"
#include "basop.h"
#include "cst_lbc.h"
#include "tab_lbc.h"
#include "lbccodec.h"
#include "coder.h"
#include "decod.h"
#include "exc_lbc.h"
#include "util_lbc.h"
#include "cod_cng.h"
#include "dec_cng.h"
#include "vad.h"
#include "tame.h"
#include "lsp.h"
#include "lpc.h"
#include "util_cng.h"

DECSTATDEF  DecStat  ;
CODCNGDEF CodCng;
CODSTATDEF  CodStat  ;
DECCNGDEF DecCng;
VADSTATDEF  VadStat ;

/* Global variables */
enum  Wmode    WrkMode = Both ;
enum  Crate    WrkRate = Rate63 ;

int   PackedFrameSize[2] = {
   24 ,
   20
   } ;

Flag    UseHp = True ;
Flag    UsePf = True ;
Flag    UseVx = False ;
Flag    UsePr = True ;

char  SignOn[] = "ACL/USH/FT/DSPG ANSI C CODEC ITU LBC Ver 5.00\n" ;

int main( int argc, char *argv[] )
{
    FILE    *Ifp, *Ofp ;            /* I/O File pointers */
    FILE    *Fep  = NULL;           /* Frame erasures file pointer */
    FILE    *Ratp = NULL;           /* Rate file pointer */

    long    FrCnt = 0 ;
    long    FlLen ;

    Word16  DataBuff[Frame] ;
    Word16  Crc ;
    char Rate_Rd;

    char    Line[24] ;

    printf("%s", SignOn ) ;

    /* Process arguments and open I/O files */
    FlLen = Process_Files( &Ifp, &Ofp, &Fep, &Ratp, argc, argv ) ;

    /*
      Init coder and the decoder
    */
    Init_Coder( ) ;
    Init_Decod( ) ;

    /* Init Comfort Noise Functions */
    if( UseVx ) {
        Init_Vad();
        Init_Cod_Cng( );
    }
    Init_Dec_Cng( );

    /* Process all the input file */
    do {

        switch ( WrkMode ) {

            case Both:
                if(Ratp != NULL) {
                    fread((char *)&Rate_Rd, sizeof(char), 1, Ratp);
                    WrkRate = (enum Crate)Rate_Rd;
                }
                if ( WrkRate == Rate53) reset_max_time();
                Read_lbc( DataBuff, Frame, Ifp ) ;
                Coder( DataBuff, Line ) ;
                Decod( DataBuff, Line, (Word16) 0 ) ;
                Write_lbc( DataBuff, Frame, Ofp ) ;
            break ;

            case Cod :
                if(Ratp != NULL) {
                    fread((char *)&Rate_Rd, sizeof(char), 1, Ratp);
                    WrkRate = (enum Crate)Rate_Rd;
                }
                if ( WrkRate == Rate53) reset_max_time();
                Read_lbc( DataBuff, Frame, Ifp ) ;
                Coder( DataBuff, Line ) ;
                Line_Wr( Line, Ofp ) ;
            break ;

            case Dec :
                if(Line_Rd( Line, Ifp ) == (-1)) {
                    FlLen = FrCnt;
                    break;
                }
                if ( Fep == NULL )
                    Crc = (Word16) 0 ;
                else
                    fread( (char *)&Crc, sizeof(Word16), 1, Fep ) ;
                Decod( DataBuff, Line, Crc ) ;
                Write_lbc( DataBuff, Frame, Ofp ) ;
            break ;
        }

        FrCnt ++ ;
        if( UsePr) {
            if( WrkMode == Dec) {
                if(FrCnt < FlLen) {
                    fprintf( stdout, "Done : %6ld\r", FrCnt) ;
                }
            }
            else {
                fprintf( stdout, "Done : %6ld %3ld\r", FrCnt, FrCnt*100/FlLen ) ;
            }
            fflush(stdout);
        }

    }   while ( FrCnt < FlLen ) ;

    if(Ifp) { (void)fclose(Ifp); }
    if(Ofp) { (void)fclose(Ofp); }
    if(Fep) { (void)fclose(Fep); }
    if(Ratp) { (void)fclose(Ratp); }
    return 0 ;
}


/*
   This function processes the argument parameters. The function
      opens the IO files, and sets the global arguments accordingly
      to the command line parameters.
*/
long  Process_Files( FILE **Ifp, FILE **Ofp, FILE **Fep, FILE **Ratp,
                                        int Argc, char *Argv[] )
{
    int     i ;
    long    Flen ;
    char    *FerFileName = NULL ;
    char    *RateFileName = NULL ;

    /*
      Process the argument list, if any
    */
    if (Argc < 3 ) {
        printf("Usage: %s [options] inputfile outputfile \n", Argv[0]);
        exit(1);
    }

    for ( i = 1 ; i < Argc-2 ; i ++ ) {

        /* Check the coder rate */
        if ( ! strncmp( "-r", Argv[i], 2) ) {
            if ( ! strcmp("63", Argv[i]+2) ) {
                WrkRate = Rate63 ;
                continue ;
            }

            else if ( ! strcmp("53", Argv[i]+2) ) {
                WrkRate = Rate53 ;
                continue ;
            }

            else {
                RateFileName = &Argv[i][2] ;
                continue ;
            }
        }

        /* Check Working mode */
        if ( ! strcmp("-b", Argv[i]) ) {
            WrkMode = Both ;
            continue ;
        }

        if ( ! strcmp("-c", Argv[i]) ) {
            WrkMode = Cod ;
            continue ;
        }

        if ( ! strcmp("-d", Argv[i]) ) {
            WrkMode = Dec ;
            continue ;
        }

        if ( ! strcmp("-v", Argv[i]) ) {
            UseVx = True ;
            continue ;
        }

        if ( ! strcmp("-Noh", Argv[i]) ) {
            UseHp = False;
            continue ;
        }

        if ( ! strcmp("-Nop", Argv[i]) ) {
            UsePf = False;
            continue ;
        }

        if ( ! strncmp( "-f", Argv[i], 2) ) {
            FerFileName = &Argv[i][2] ;
            continue ;
        }

        if ( ! strcmp("-n", Argv[i]) ) {
            UsePr = False;
            continue ;
        }

        fprintf(stderr, "Illegal argument, %s\n", Argv[i] ) ;
        exit(1) ;
    }

    *Ifp = fopen( Argv[Argc-2], "rb") ;
    if ( *Ifp == NULL ) {
        fprintf(stderr, "Invalid input file name: %s\n", Argv[Argc-2] ) ;
        exit(1) ;
    }

    if ( UsePr )
        printf("Input  file:     %s\n", Argv[Argc-2] ) ;

    *Ofp = fopen( Argv[Argc-1], "wb") ;
    if ( *Ofp == NULL ) {
        fprintf(stderr, "Can't open output file: %s\n", Argv[Argc-1] ) ;
        exit(1) ;
    }
    if ( UsePr )
        printf("Output file:     %s\n", Argv[Argc-1] ) ;

    /* Open Fer file if required */
    if ( WrkMode == Dec ) {

        if ( FerFileName != NULL ) {
            *Fep = fopen( FerFileName, "rb" ) ;
            if ( *Fep == NULL ) {
                fprintf(stderr, "Can't open FER file: %s\n", FerFileName ) ;
                exit(1) ;
            }
            if ( UsePr )
                printf("FER    file:     %s\n", FerFileName ) ;
        }
    }
    else {
        if ( RateFileName != NULL ) {
            *Ratp = fopen( RateFileName, "rb" ) ;
            if ( *Ratp == NULL ) {
                fprintf(stderr, "Can't open Rate file: %s\n", RateFileName ) ;
                exit(1) ;
            }
            if ( UsePr )
                printf("Rate   file:     %s\n", RateFileName ) ;
        }
    }

    /* Options report */
    if ( UsePr ) {

        printf("Options:\n");
        if (WrkMode == Both )
            printf("Encoder/Decoder\n");
        else if (WrkMode == Cod )
            printf("Encoder\n");
        else
            printf("Decoder\n");


        if( WrkMode != Cod ) {
            if (UsePf == 0 )
                printf("Postfilter disabled\n");
            else
                printf("Postfilter enabled\n");
        }

        if( WrkMode <= Cod ) {
            if(*Ratp == NULL) {
                if (WrkRate == Rate63 )
                    printf("Rate 6.3 kb/s\n");
                else
                    printf("Rate 5.3 kb/s\n");
            }
            if (UseHp == 0 )
                printf("Highpassfilter disabled\n");
            else
                printf("Highpassfilter enabled\n");
            if (UseVx == 0 )
                printf("VAD/CNG disabled\n");
            else
                printf("VAD/CNG enabled\n");
        }
    }

    /*
      Compute the file length
    */
    fseek( *Ifp, 0L, SEEK_END ) ;
    Flen = ftell( *Ifp ) ;
    rewind( *Ifp ) ;
    if ( WrkMode == Dec )
        Flen = 0x7fffffffL ;
    else
        Flen /= sizeof(Word16)*Frame ;

    return Flen ;
}


#include "basop.c"
#include "tab_lbc.c"
#include "coder.c"
#include "decod.c"
#include "exc_lbc.c"
#include "util_lbc.c"
#include "cod_cng.c"
#include "dec_cng.c"
#include "vad.c"
#include "tame.c"
#include "lsp.c"
#include "lpc.c"
#include "util_cng.c"