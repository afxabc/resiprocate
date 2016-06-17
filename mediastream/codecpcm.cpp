#include "codecpcm.h"

CodecPcm::CodecPcm(unsigned char pt):type(pt)
{
	ms_bufferizer_init(&inData);
}

CodecPcm::~CodecPcm()
{
	ms_bufferizer_uninit(&inData);
}

void CodecPcm::encode(queue_t *payload)
{
	ms_bufferizer_put_from_queue(&inData, payload);
	while(ms_bufferizer_get_avail(&inData)>=320)
	{
		uint8_t data[320];
		ms_bufferizer_read(&inData, (uint8_t*)data, 320);
		int len = 160;
		mblk_t *ulawbuf = allocb(len,0);
		for (int i=0;i<len;i++)
		{
			if(type == CodecNo_PCMU)
				*ulawbuf->b_wptr=s16_to_ulaw(((__int16*)data)[i]);
			else
				*ulawbuf->b_wptr=s16_to_alaw(((__int16*)data)[i]);
			ulawbuf->b_wptr++;
		}

		mblk_set_payload_type(ulawbuf,type);
		mblk_set_timestamp_info(ulawbuf,len);
		putq(payload,ulawbuf);
	}
}

mblk_t* CodecPcm::decode(mblk_t *inm)
{
	int len = msgdsize(inm);
	mblk_t *om = allocb(len*2,0);
	for(int i=0; i<len; i++)
	{
		if(type == CodecNo_PCMU)
			*((__int16*)(om->b_wptr))=ulaw_to_s16(*(inm->b_rptr+i));
		else
			*((__int16*)(om->b_wptr))=alaw_to_s16(*(inm->b_rptr+i));
		om->b_wptr+=2;
	}
	return om;
}