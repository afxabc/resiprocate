#include "codecg723.h"

CodecG723::CodecG723()
{
	ms_bufferizer_init(&inData);
	ms_bufferizer_init(&outData);
    Init_Coder( ) ;
    Init_Decod( ) ;

    /* Init Comfort Noise Functions */
    if( 1 ) {
        Init_Vad();
        Init_Cod_Cng( );
    }
    Init_Dec_Cng( );
}

CodecG723::~CodecG723()
{
	ms_bufferizer_uninit(&inData);
	ms_bufferizer_uninit(&outData);
}

void CodecG723::encode(queue_t *payload)
{
	ms_bufferizer_put_from_queue(&inData, payload);
	while(ms_bufferizer_get_avail(&inData) >= Frame*2)
	{
		char *input = new char[Frame*2];
		ms_bufferizer_read(&inData, (uint8_t*)input, Frame*2);

		int len;
		mblk_t *om=allocb(24,0);
//		reset_max_time();
		Coder((Word16*)input, (char*)om->b_wptr);
		char tmp=*(om->b_rptr)&0x03;
		if(tmp==0)
			len=24;
		if(tmp==1)
			len=20;
		if(tmp==2)
			len=4;
		if(tmp==3)
			len=1;
		om->b_wptr += len;

		mblk_set_payload_type(om, CodecNo_G723);
		mblk_set_timestamp_info(om, Frame);
		putq(payload, om);
	}
}

mblk_t* CodecG723::decode(mblk_t *inm)
{
	mblk_t *om = NULL;
/*	char tmp = *(inm->b_rptr) & 0x03;
	if((tmp == 00)||(tmp==01))
	{
		om=allocb(Frame*2,0);
		Decod((Word16*)om->b_wptr, (char*)inm->b_rptr, (Word16) 0 ) ;
		om->b_wptr += Frame*2;
	}*/
	char tmp = *(inm->b_rptr) & 0x03;
	if((tmp == 00)||(tmp==01))
	{
		mblk_t *inm1=dupmsg(inm);
		ms_bufferizer_put(&outData, inm1);

		int pktnum = ms_bufferizer_get_avail(&outData);
		pktnum = pktnum/24;
		if(pktnum > 0)
			om = allocb(480*pktnum, 0);
		for(int k=0;k<pktnum;k++)
		{
			uint8_t data[24];
			ms_bufferizer_read(&outData, (uint8_t*)data, 24);
			Decod((Word16*)om->b_wptr, (char*)data, (Word16) 0 ) ;
			om->b_wptr += Frame*2;
		}
	}
	return om;
}