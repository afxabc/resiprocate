#include "codecgsm.h"

CodecGsm::CodecGsm()
{
	gsm_state = gsm_create();
	ms_bufferizer_init(&inData);
}

CodecGsm::~CodecGsm()
{
	free((void*)gsm_state);
	ms_bufferizer_uninit(&inData);
}

void CodecGsm::encode(queue_t *payload)
{
	ms_bufferizer_put_from_queue(&inData, payload);
	while(ms_bufferizer_get_avail(&inData)>=320)
	{
		uint8_t data[320];
		ms_bufferizer_read(&inData, (uint8_t*)data, 320);

		mblk_t *om=allocb(33,0);
		gsm_encode(gsm_state,(gsm_signal*)data,(gsm_byte*)om->b_wptr);
		om->b_wptr+=33;
				
		mblk_set_payload_type(om, CodecNo_GSM);
		mblk_set_timestamp_info(om,160);
		putq(payload,om);
	}
}

mblk_t* CodecGsm::decode(mblk_t *inm)
{
	mblk_t *om=allocb(320,0);
	gsm_decode(gsm_state,(gsm_byte*)inm->b_rptr,(gsm_signal*)om->b_wptr);
	om->b_wptr+=320;
	return om;
}