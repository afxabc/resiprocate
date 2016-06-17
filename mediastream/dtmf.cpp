#include "Dtmf.h"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace resip;

Dtmf::Dtmf()
{
	current_tev = NULL;
}

Dtmf::~Dtmf() {
}

int Dtmf::Dtmf_send(RTPSession *session, char dtmf)
{
	mblk_t *m1,*m2,*m3;
	int tev_type;
	int durationtier = 480/3;

	/* create the first telephony event packet */
	switch (dtmf){
		case '1':
			tev_type=TEV_DTMF_1;
		break;
		case '2':
			tev_type=TEV_DTMF_2;
		break;
		case '3':
			tev_type=TEV_DTMF_3;
		break;
		case '4':
			tev_type=TEV_DTMF_4;
		break;
		case '5':
			tev_type=TEV_DTMF_5;
		break;
		case '6':
			tev_type=TEV_DTMF_6;
		break;
		case '7':
			tev_type=TEV_DTMF_7;
		break;
		case '8':
			tev_type=TEV_DTMF_8;
		break;
		case '9':
			tev_type=TEV_DTMF_9;
		break;
		case '*':
			tev_type=TEV_DTMF_STAR;
		break;
		case '0':
			tev_type=TEV_DTMF_0;
		break;
		case '#':
			tev_type=TEV_DTMF_POUND;
		break;

		case 'A':
		case 'a':
			tev_type=TEV_DTMF_A;
			break;


		case 'B':
		case 'b':
			tev_type=TEV_DTMF_B;
			break;

		case 'C':
		case 'c':
		  tev_type=TEV_DTMF_C;
		  break;

		case 'D':
		case 'd':
		  tev_type=TEV_DTMF_D;
		  break;

		case '!':
		  tev_type=TEV_FLASH;
		  break;


		default:
			InfoLog (<<"Bad dtmf: "<<dtmf);
			return -1;
	}

	m1=Dtmf_create_event(tev_type,0,10,durationtier);
	/* create a second packet */
	m2=Dtmf_create_event(tev_type,0,10, durationtier+durationtier);
		
	/* create a third and final packet */
	m3=Dtmf_create_event(tev_type,1,10,480);
	
	/* and now sends them */
	session->SendPacket((void *)m1->b_rptr,msgdsize(m1),(unsigned char)TELEPHONE_EVENT,true,(unsigned long)0);
	session->SendPacket((void *)m2->b_rptr,msgdsize(m2),(unsigned char)TELEPHONE_EVENT,false,(unsigned long)0);
	session->SendPacket((void *)m3->b_rptr,msgdsize(m3),(unsigned char)TELEPHONE_EVENT,false,(unsigned long)0);
	session->SendPacket((void *)m3->b_rptr,msgdsize(m3),(unsigned char)TELEPHONE_EVENT,false,(unsigned long)0);
	session->SendPacket((void *)m3->b_rptr,msgdsize(m3),(unsigned char)TELEPHONE_EVENT,false,(unsigned long)480);
	/* the last packet is sent three times in order to improve reliability*/
	freemsg(m1);
	freemsg(m2);
	freemsg(m3);

	return 0;
}


/**
 *@param session a rtp session.
 *@param packet a rtp packet as a mblk_t
 *@param event the event type as described in rfc2833, ie one of the TEV_* macros.
 *@param end a boolean to indicate if the end bit should be set. (end of tone)
 *@param volume the volume of the telephony tone, as described in rfc2833
 *@param duration the duration of the telephony tone, in timestamp unit.
 *
 * Adds a named telephony event to a rtp packet previously allocated using
 * rtp_session_create_telephone_event_packet().
 *
 *@return 0 on success.
**/
mblk_t* Dtmf::Dtmf_create_event(unsigned __int8 evt, int end, unsigned __int8 volume, unsigned __int16 duration)
{
	telephone_event_t *event_hdr;
	mblk_t *mp=allocb(TELEPHONY_EVENTS_ALLOCATED_SIZE,0);

	event_hdr=(telephone_event_t*)mp->b_wptr;
	event_hdr->evt=evt;
	event_hdr->R=0;
	event_hdr->E=end;
	event_hdr->volume=volume;
	event_hdr->duration=(duration);
	mp->b_wptr+=sizeof(telephone_event_t);
	return mp;
}


void Dtmf::process(MediaContext &mc)
{
	mblk_t *inm;
	queue_t payload;

	qinit(&payload);

	while(inm=getq(&mc.payload0))
	{
		if(mblk_get_payload_type(inm) == TELEPHONE_EVENT)
		{
			Dtmf_parse(inm, &payload);
		}
		else
		{
			putq(&payload,inm);
		}
	}
	while(inm=getq(&payload))
	{
		putq(&mc.payload0,inm);
	}
}

void Dtmf::Dtmf_parse(mblk_t *m0, queue_t *payload)
{
	int datasize;
	int num,num2;
	telephone_event_t *events,*evbuf;

	datasize=msgdsize(m0);
	num=datasize/sizeof(telephone_event_t);
	events=(telephone_event_t*)m0->b_rptr;

	if (mblk_get_marker_info(m0)==1)
	{
		/* this is a start of new events. Store the event buffer for later use*/
		if (current_tev!=NULL) {
			freemsg(current_tev);
			current_tev=NULL;
		}
		current_tev=copymsg(m0);
		current_tev->reserved1 = m0->reserved1;
		current_tev->reserved2 = m0->reserved2;
		/* handle the case where the events are short enough to end within the packet that has the marker bit*/
		notify_events_ended(events,num,payload);
	}

	if (current_tev!=NULL)
	{
		/* first compare timestamp, they must be identical */
		if (mblk_get_timestamp_info(current_tev)==mblk_get_timestamp_info(m0))
		{
			datasize=msgdsize(current_tev);
			num2=datasize/sizeof(telephone_event_t);
			evbuf=(telephone_event_t*)current_tev->b_rptr;
			for (int i=0;i<MIN(num,num2);i++)
			{
				if (events[i].E==1)
				{
					/* update events that have ended */
					if (evbuf[i].E==0){
						evbuf[i].E=1;
						/* this is a end of event, report it */
						notify_tev(&events[i],payload);
					}
				}
			}
		}
		else
		{
			/* timestamp are not identical: this is not the same events*/
			if (current_tev!=NULL) {
				freemsg(current_tev);
				current_tev=NULL;
			}
			current_tev=copymsg(m0);
			current_tev->reserved1 = m0->reserved1;
			current_tev->reserved2 = m0->reserved2;
			notify_events_ended(events,num,payload);
		}
	}
	else
	{
		/* there is no pending events, but we did not received marked bit packet
		either the sending implementation is not compliant, either it has been lost, 
		we must deal with it anyway.*/
		current_tev=copymsg(m0);
		current_tev->reserved1 = m0->reserved1;
		current_tev->reserved2 = m0->reserved2;
		/* inform the application if there are tone ends */
		notify_events_ended(events,num,payload);
	}
}

void Dtmf::notify_tev(telephone_event_t *evt, queue_t *payload)
{
	InfoLog (<<"event = "<<evt[0].evt);
	float highfreq;
	float lowfreq;
	switch(evt[0].evt)
	{
		case TEV_DTMF_0:
			lowfreq=941;
			highfreq=1336;
			break;
		case TEV_DTMF_1:
			lowfreq=697;
			highfreq=1209;
			break;
		case TEV_DTMF_2:
			lowfreq=697;
			highfreq=1336;
			break;
		case TEV_DTMF_3:
			lowfreq=697;
			highfreq=1477;
			break;
		case TEV_DTMF_4:
			lowfreq=770;
			highfreq=1209;
			break;
		case TEV_DTMF_5:
			lowfreq=770;
			highfreq=1336;
			break;
		case TEV_DTMF_6:
			lowfreq=770;
			highfreq=1477;
			break;
		case TEV_DTMF_7:
			lowfreq=852;
			highfreq=1209;
			break;
		case TEV_DTMF_8:
			lowfreq=852;
			highfreq=1336;
			break;
		case TEV_DTMF_9:
			lowfreq=852;
			highfreq=1477;
			break;
		case TEV_DTMF_STAR:
			lowfreq=941;
			highfreq=1209;
			break;
		case TEV_DTMF_POUND:
			lowfreq=941;
			highfreq=1477;
			break;
		case TEV_DTMF_A:
			lowfreq=697;
			highfreq=1633;
			break;
		case TEV_DTMF_B:
			lowfreq=770;
			highfreq=1633;
			break;
		case TEV_DTMF_C:
			lowfreq=852;
			highfreq=1633;
			break;
		case TEV_DTMF_D:
			lowfreq=941;
			highfreq=1633;
			break;	
		default:
			return;
	}
	lowfreq /= 8000;
	highfreq /= 8000;
	mblk_t *m = allocb(1600, 0);
	__int16 *sample=(__int16*)m->b_wptr;
	for(int i=0;i<800;i++)
	{
		sample[i]=(__int16)(10000.0*sin(2*M_PI*(float)i*lowfreq));
		sample[i]+=(__int16)(10000.0*sin(2*M_PI*(float)i*highfreq));
	}
	m->b_wptr += 1600;
	mblk_set_payload_type(m, TELEPHONE_EVENT);
	putq(payload,m);
}

void Dtmf::notify_events_ended(telephone_event_t *events, int num, queue_t *payload)
{
	int i;
	for (i=0;i<num;i++){
		if (events[i].E==1){
			notify_tev(&events[i],payload);
		}
	}
}