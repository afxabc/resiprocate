
#include "mssndcard.h"
#include "WincardRead.h"
#include "WincardWrite.h"
//#include "./videostream/WinVideo.h"
#include "Dtmf.h"
#include <Vfw.h>

using namespace std;

MSSndCard::MSSndCard(): spklevel(0xdfff),
spkmute(false),
miclevel(0xdfff),
micmute(false),
play(NULL),
devid(0)
{
	MSSndCardDetect();
	MSVideoDevDetect();
}

MSSndCard::~MSSndCard()
{
	mCapCards.clear();
	mPlayCards.clear();

	mVideoDevs.clear();
}

int MSSndCard::GetAudioCapDevTotal()
{
	return mCapCards.size();
}

int MSSndCard::GetAudioPlayDevTotal()
{
	return mPlayCards.size();
}

int MSSndCard::GetVideoDevTotal()
{
	return mVideoDevs.size();
}

void MSSndCard::MSSndCardDetect()
{
	 MMRESULT mr = NOERROR;
	 unsigned int nInDevices = waveInGetNumDevs ();
	 unsigned int nOutDevices=waveOutGetNumDevs ();
	 unsigned int item;

/*	 if (nOutDevices>nInDevices)
	 {
		 nInDevices = nOutDevices;
	 }*/
	 for(item = 0; item < nInDevices; item++)
	 {
		 WAVEINCAPSA incaps;
		 mr = waveInGetDevCapsA (item, &incaps, sizeof (WAVEINCAPSA));
		 if (mr == MMSYSERR_NOERROR)
		 {
			 AddCapCardName(item,incaps.szPname);
		 }
	 }
	 for(item = 0; item < nOutDevices; item++)
	 { 
		 WAVEOUTCAPSA outcaps;
		 mr = waveOutGetDevCapsA (item, &outcaps, sizeof (WAVEOUTCAPSA));
		 if (mr == MMSYSERR_NOERROR)
		 {
			 AddPlayCardName(item,outcaps.szPname);
		 }
	 }
	 
}

void MSSndCard::MSVideoDevDetect()
{
	int i;
	char dev[80];
	char ver[80];
	
	for (i = 0; i < 9; i++)
	{
		if (capGetDriverDescriptionA(i, dev, sizeof (dev),
			ver, sizeof (ver)))
		{
			mVideoDevs[i] = Data(dev);
		}
		else
		{
			break;
		}
	}
}

void MSSndCard::CapCardSetLevel(int cardno, int percent)
{
}

void MSSndCard::PlayCardSetLevel(int cardno, int percent)
{
}

void MSSndCard::CapCardSetCapture(int cardno)
{
}

void MSSndCard::PlayCardSetCapture(int cardno)
{
}

int MSSndCard::CapCardGetLevel(int cardno)
{
	return 0;
}

int MSSndCard::PlayCardGetLevel(int cardnoe)
{
	return 0;
}

void MSSndCard::AddCapCardName(int cardno,char* name)
{
	mCapCards[cardno] = Data(name);
}

void MSSndCard::AddPlayCardName(int cardno,char* name)
{
	mPlayCards[cardno] = Data(name);
}

resip::Data MSSndCard::GetCapCardName(int cardno)
{
	return mCapCards[cardno];
}

resip::Data MSSndCard::GetVideoCapName(int cardno)
{
	return mVideoDevs[cardno];
}

resip::Data MSSndCard::GetPlayCardName(int cardno)
{
	return mPlayCards[cardno];
}

void MSSndCard::SetDTMFValue(resip::Data& dt)
{
	if( DTMFValue == resip::Data::Empty )
	{
		DTMFValue = dt;
	}
}

void MSSndCard::PlayDTMF()
{
	if(play != NULL)
	{
		return;
	}
	play=new WincardWrite(devid);
	play->card_detect();
	play->card_init();
	play->cardwrite_preprocess();

	MediaContext mc;
	Dtmf dtmf;
	telephone_event_t televt;
	televt.evt=DTMFValue.convertInt();
	dtmf.notify_tev((telephone_event_t*)&televt,(queue_t*)&mc.payload0);

	play->process(mc);
	
	DTMFValue=resip::Data::Empty;
}

void MSSndCard::process()
{
	if(DTMFValue!=resip::Data::Empty)
	{
		PlayDTMF();
	}
	if(play!=NULL)
	{
		if(play->isPlayComplete())
		{
			delete play;
			play=NULL;
		}
	}
}
