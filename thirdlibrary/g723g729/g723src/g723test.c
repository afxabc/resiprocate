#include "typedef.h"
#include "basop_t.h"

main()
{
	Word16	out;
	Word32	in = 0x11112222;
	out=sature(in);
	return ;
}

#include "basop_t.c"