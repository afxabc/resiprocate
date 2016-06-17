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

main()
{
	Word32 in;
	Word16 out;

	in=(Word32)0x11110022;
	out=sature(in);
	printf("0x11110022 : %x --- ov=%d\n",out,Overflow);

	in=(Word32)0x80000022;
	out=sature(in);
	printf("0x80000022 : %x --- ov=%d\n",out,Overflow);

	in=(Word32)0xffff8000;
	out=sature(in);
	printf("0xffff8000 : %x --- ov=%d\n",out,Overflow);

	in=(Word32)0x000;
	out=sature(in);
	printf("0x00000000 : %x --- ov=%d\n",out,Overflow);

	in=(Word32)0x00007fff;
	out=sature(in);
	printf("0x00007fff : %x --- ov=%d\n",out,Overflow);

	in=(Word32)0xffff8100;
	out=sature(in);
	printf("0xffff8100 : %x --- ov=%d\n",out,Overflow);

	in=(Word32)0x00007000;
	out=sature(in);
	printf("0x00007000 : %x --- ov=%d\n",out,Overflow);

	printf("\n-------------------------------------------\n");

	printf("%lx  %lx\n",(Word32)0x0ffff7001,extract_l((Word32)0x0ffff7001));
}


