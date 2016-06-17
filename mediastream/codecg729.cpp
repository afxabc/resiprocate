#include "codecg729.h"

CodecG729::CodecG729():frame(0)
{
	ms_bufferizer_init(&inData);
	ms_bufferizer_init(&outData);
	//init coder
	Init_Pre_Process();
	Init_Coder_ld8k();
	Init_Cod_cng();
	for(Word16 i=0; i<PRM_SIZE; i++)
		prm[i] = (Word16)0;

	//init decoder
	for (int i=0; i<M; i++) 
		synth_buf[i] = 0;
	synth = synth_buf + M;
	Init_Decod_ld8k();
	Init_Post_Filter();
	Init_Post_Process();
	voicing = 60;
	/* for G.729b */
	Init_Dec_cng();
}

CodecG729::~CodecG729()
{
	ms_bufferizer_uninit(&inData);
	ms_bufferizer_uninit(&outData);
}

void CodecG729::encode(queue_t *payload)
{
	
	ms_bufferizer_put_from_queue(&inData, payload);
	while(ms_bufferizer_get_avail(&inData) >= L_FRAME*4)
	{
		mblk_t *om=allocb(20,0);
		for (int k=0;k<2;k++)
		{
			ms_bufferizer_read(&inData, (uint8_t*)GetNewSpeech(), L_FRAME*2);
			if (frame == 32767)
				frame = 256;
			else 
				frame++;
			Pre_Process(GetNewSpeech(), L_FRAME);
			Coder_ld8k(prm, syn, frame, 0);
			prm2bits_ld8k( prm, cserial);
			for(int i = 0; i<cserial[1]; i++)
			{
				*om->b_wptr <<= 1;
				if(cserial[i+2] == BIT_1)
					*om->b_wptr += 1;
				if((i+1)%8 == 0)
					om->b_wptr++;
			}
			
		}
		mblk_set_payload_type(om, CodecNo_G729);
		mblk_set_timestamp_info(om, L_FRAME*2);
		putq(payload, om);
	}
	
}

mblk_t* CodecG729::decode(mblk_t *inm)
{
	mblk_t *inm1=dupmsg(inm);
	mblk_t *om=NULL;
	ms_bufferizer_put(&outData, inm1);
	
	int pktnum = ms_bufferizer_get_avail(&outData);
	pktnum = pktnum/10;
	if(pktnum > 0)
		om = allocb(160*pktnum, 0);
	for(int k=0;k<pktnum;k++)
	{
		uint8_t tmp[10];
		ms_bufferizer_read(&outData, tmp, 10);

		for(int i=0;i<10;i++)
		{
			for(int j=0;j<8;j++)
			{
				if(((tmp[i] >> (7-j))&0x01) == 1)
					dserial[2+i*8+j] = BIT_1;
				else
					dserial[2+i*8+j] = BIT_0;
			}
		}
		dserial[0]=SYNC_WORD;
		dserial[1]=80;
		bits2prm_ld8k(&dserial[1], parm);
		parm[0] = 0;           /* No frame erasure */
		if(dserial[1] != 0) 
		{
			for (int i=0; i < dserial[1]; i++)
			if (dserial[i+2] == 0 ) parm[0] = 1;  /* frame erased     */
		}
		else if(dserial[0] != SYNC_WORD) parm[0] = 1;
		if(parm[1] == 1) 
		{
			/* check parity and put 1 in parm[5] if parity error */
			parm[5] = Check_Parity_Pitch(parm[4], parm[5]);
		}
		Decod_ld8k(parm, voicing, synth, Az_dec, &T0_first, &Vad);
		voicing = 0;
		ptr_Az = Az_dec;
		for(int i=0; i<L_FRAME; i+=L_SUBFR) 
		{
			Post(T0_first, &synth[i], ptr_Az, &pst_out[i], &sf_voic, Vad);
			if (sf_voic != 0) { voicing = sf_voic;}
			ptr_Az += MP1;
		}
		Copy(&synth_buf[L_FRAME], &synth_buf[0], M);
		    
		Post_Process(pst_out, L_FRAME);
			
		memcpy(om->b_wptr, (char*)pst_out, L_FRAME*2);
		om->b_wptr += L_FRAME*2;
		
	}
	return om;
}