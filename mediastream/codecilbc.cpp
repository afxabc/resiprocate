#include "codecilbc.h"

CodeciLBC::CodeciLBC()
{
	initEncode(&Enc_Inst, 30);
	initDecode(&Dec_Inst, 30, 1);
	ms_bufferizer_init(&inData);
	ms_bufferizer_init(&outData);
}

CodeciLBC::~CodeciLBC()
{
	ms_bufferizer_uninit(&inData);
	ms_bufferizer_uninit(&outData);
}

void CodeciLBC::encode(queue_t *payload)
{
	ms_bufferizer_put_from_queue(&inData, payload);
	while(ms_bufferizer_get_avail(&inData) >= BLOCKL_30MS*2)
	{
		short data[BLOCKL_30MS];
		float block[BLOCKL_30MS];
		ms_bufferizer_read(&inData, (uint8_t*)data, BLOCKL_30MS*2);
					
		/* convert signal to float */
		for (int k=0; k<BLOCKL_30MS; k++)
			block[k] = (float)data[k];
	
		unsigned char encoded_data[NO_OF_BYTES_30MS];
		iLBC_encode(encoded_data, block, &Enc_Inst);

		mblk_t *alawbuf = allocb(NO_OF_BYTES_30MS,0);
		memcpy(alawbuf->b_wptr, encoded_data, NO_OF_BYTES_30MS);
		alawbuf->b_wptr+=NO_OF_BYTES_30MS;

		mblk_set_payload_type(alawbuf, CodecNo_iLBC);
		mblk_set_timestamp_info(alawbuf,BLOCKL_30MS);
		putq(payload,alawbuf);
	}
}

mblk_t* CodeciLBC::decode(mblk_t *inm)
{
	float decblock[BLOCKL_30MS], dtmp;
	short decoded_data[BLOCKL_30MS];
	int len = msgdsize(inm);

	iLBC_decode(decblock, (unsigned char*)inm->b_rptr, &Dec_Inst,1);

	for (int k=0; k<BLOCKL_30MS; k++)
	{
		dtmp=decblock[k];
		if (dtmp<MIN_SAMPLE)
			dtmp=MIN_SAMPLE;
		else if (dtmp>MAX_SAMPLE)
			dtmp=MAX_SAMPLE;
		decoded_data[k] = (short) dtmp;
	}

	mblk_t *om = allocb(BLOCKL_30MS*2,0);
	memcpy((unsigned char*)om->b_wptr, (unsigned char*)decoded_data, BLOCKL_30MS*2);
	om->b_wptr += BLOCKL_30MS*2;

	return om;
	/*
	mblk_t *alawbuf = allocb(BLOCKL_30MS*2,0);
	memcpy((unsigned char*)alawbuf->b_wptr, (unsigned char*)decoded_data, BLOCKL_30MS*2);
	alawbuf->b_wptr+=BLOCKL_30MS*2;

	unsigned char data[320];
	ms_bufferizer_put(&outData, alawbuf);
	while(ms_bufferizer_read(&outData, (uint8_t*)data, 320) > 0)
	{

		mblk_t *alawbuf1 = allocb(320,0);
		memcpy((unsigned char*)alawbuf1->b_wptr, (unsigned char*)data, 320);
		alawbuf1->b_wptr+=320;

		putq(payload,alawbuf1);
	}*/
}