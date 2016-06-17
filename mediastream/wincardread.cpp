/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "wincardread.h"
#include "rutil/Lock.hxx"
#include "mssndcard.h"
#include "audioconf.h"
#include <iostream>
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace std;
using namespace resip;


WincardRead::WincardRead(int devid)
{
	dev_id=devid;

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.cbSize = 0;/*PCM格式时，忽略*/
	wfx.nAvgBytesPerSec = 16000;/*数据传输速率，字节/秒*/
	wfx.nBlockAlign = 2;/*每个样本的字节数*/
	wfx.nChannels = 1;/*通道数*/
	wfx.nSamplesPerSec = 8000;/*采样率*/
	wfx.wBitsPerSample = 16;/*每个样本的bit数*/

	level = 1;
	mute = 1;
	peak = 0;

	qinit(&rq);
}

WincardRead::WincardRead(const WincardRead& card):dev_id(card.dev_id)
{
	wfx.wFormatTag = card.wfx.wFormatTag;
	wfx.cbSize = card.wfx.cbSize;
	wfx.nAvgBytesPerSec = card.wfx.nAvgBytesPerSec;
	wfx.nBlockAlign = card.wfx.nBlockAlign;
	wfx.nChannels = card.wfx.nChannels;
	wfx.nSamplesPerSec = card.wfx.nSamplesPerSec;
	wfx.wBitsPerSample = card.wfx.wBitsPerSample;

	qinit(&rq);
}

WincardRead::~WincardRead()
{
	card_read_postprocess();
}

void WincardRead::card_detect()
{
    MMRESULT mr = NOERROR;
	unsigned int nInDevices = waveInGetNumDevs ();
	unsigned int item;
    
	for(item = 0; item < nInDevices; item++)
	{
		WAVEINCAPSA incaps;
		mr = waveInGetDevCapsA (item, &incaps, sizeof (WAVEINCAPSA));
		if (mr == MMSYSERR_NOERROR)
		{
//			parent->CardCreateReader(item);
//			parent->SetCapCardName(item,incaps.szPname);
		}
	}
}


void WincardRead::card_init()
{
	wfx.nBlockAlign = wfx.wBitsPerSample/8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec*wfx.nBlockAlign;
	outcurbuf = 0;
	nsamples = 0;
	
	stat_minimumbuffer = WINSND_MINIMUMBUFFER;

	begin = false;
	bytes_read = 0;
}


mblk_t* WincardRead::readBuffer()
{
	Lock lock(mutex); (void)lock;
	mblk_t* m = getq(&rq);
	return m;
}

void WincardRead::process(MediaContext &mc)
{
	mblk_t* m;
	
	if(mc.payload0.q_mcount>0)
	{
		flushq(&mc.payload0,0);
	}
	while(1)
	{
		m=readBuffer();
		if(m==NULL)
		{
			break;
		}

		/* volume control */
		int len = msgdsize(m)/2;
		short *tmp = (short*)m->b_rptr;
		unsigned int tmp_peak = 0;
		float tmp_level = level;
		for(int i=0;i<len;i++)
		{
			tmp[i] *= mute;
			int comp=tmp_peak - abs(tmp[i]);
			if(comp<0)
			{
				tmp_peak=abs(tmp[i]);
			}
			tmp[i] *= tmp_level;
		}
//		tmp_peak /= len;
		peak = tmp_peak;
/*
		int size = conf->getTotalMember();
		char mixer[1280];
		memset(mixer, 0, sizeof(mixer));
		conf->readBufferizer(pid, mixer, msgdsize(m));
		for(int j=0; j<len; j++)
		{
			tmp[j] = tmp[j] / size;
			tmp[j] += ((short*)mixer)[j];
		}*/

		putq(&mc.payload0,m);
	}
	
	for(int i = 0;i < WINSND_NBUFS; ++i)
	{
		WAVEHDR *hdr = &hdrs_read[i];
		if (hdr->dwUser == 0)
		{
			MMRESULT mr;
			mr = waveInUnprepareHeader(indev,hdr,sizeof(*hdr));
			if (mr != MMSYSERR_NOERROR)
				cout<<"winsnd_read_process: Fail to unprepare header!"<<endl;
			add_input_buffer(hdr,hdr->dwBufferLength);
		}
	}
	
}

void WincardRead::add_input_buffer(WAVEHDR *hdr, int buflen)
{
	mblk_t* m = allocb(buflen,0);
	MMRESULT mr;
	memset(hdr,0,sizeof(*hdr));
	if (buflen == 0) cout<<"add_input_buffer: buflen=0 !"<<endl;

	hdr->lpData = (LPSTR)m->b_wptr;
	hdr->dwBufferLength = buflen;
	hdr->dwFlags = 0;
	hdr->dwUser = (DWORD)m;

	mr = waveInPrepareHeader (indev,hdr,sizeof(*hdr));
	if (mr != MMSYSERR_NOERROR)
	{
		cout<<"waveInPrepareHeader() error"<<endl;
		return ;
	}
	mr = waveInAddBuffer(indev,hdr,sizeof(*hdr));
	if (mr != MMSYSERR_NOERROR)
	{
		cout<<"waveInAddBuffer() error"<<endl;
		return ;
	}
}

void CALLBACK
read_callback(HWAVEIN waveindev, UINT uMsg, DWORD dwInstance, DWORD dwParam1,
                DWORD dwParam2)
{
	WAVEHDR *wHdr = (WAVEHDR *)dwParam1;
	mblk_t* m;
	WincardRead *wr = (WincardRead*)dwInstance;
	int bsize;
	switch (uMsg){
		case WIM_OPEN:
			cout<<"read_callback : WIM_OPEN"<<endl;
			break;
		case WIM_CLOSE:
			cout<<"read_callback : WIM_CLOSE"<<endl;
			break;
		case WIM_DATA:
			bsize = wHdr->dwBytesRecorded;
			/* ms_warning("read_callback : WIM_DATA (%p,%i)",wHdr,bsize); */
			m = (mblk_t*)wHdr->dwUser;
			m->b_wptr += bsize;
			wHdr->dwUser = 0;

			Lock lock(wr->mutex); (void)lock;
			putq(&wr->rq, m);
			wr->bytes_read += wHdr->dwBufferLength;
			break;
	}
}

void WincardRead::card_read_close()
{
	MMRESULT mr;
	int i;

	mr = waveInStop(indev);

	mr = waveInReset(indev);

	for(i = 0; i < WINSND_NBUFS; ++i)
	{
		WAVEHDR *hdr = &hdrs_read[i];
		if (hdr->dwFlags & WHDR_PREPARED)
		{
			mr = waveInUnprepareHeader(indev,hdr,sizeof (*hdr));
			if (mr != MMSYSERR_NOERROR)
			{
				cout<<"waveInUnPrepareHeader() error"<<endl;
			}
		}
	}
	mr = waveInClose(indev);
	if (mr != MMSYSERR_NOERROR)
	{
		cout<<"waveInClose() error"<<endl;
		return ;
	}
	flushq(&rq,0);
}

void WincardRead::card_read_preprocess()
{
	MMRESULT mr;
	int i;
	int bsize;
	DWORD dwFlag;

	stat_minimumbuffer = WINSND_MINIMUMBUFFER;

	wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	/* Init Microphone device */
	dwFlag = CALLBACK_FUNCTION;
	if (dev_id != WAVE_MAPPER)
	{
		dwFlag = WAVE_MAPPED | CALLBACK_FUNCTION;
	}
	mr = waveInOpen (&indev, dev_id, &wfx,(DWORD)read_callback,(DWORD)this, dwFlag);
	if (mr != MMSYSERR_NOERROR)
	{
	    ErrLog(<<"Failed to prepare windows sound device. ");
	    mr = waveInOpen (&indev, WAVE_MAPPER, &wfx,(DWORD)read_callback,(DWORD)this, CALLBACK_FUNCTION);
		if (mr != MMSYSERR_NOERROR)
		{
			indev=NULL;
			ErrLog(<<"Failed to prepare windows sound device. ");
		    return ;
		}
	}
	bsize = WINSND_NSAMPLES * wfx.nAvgBytesPerSec / 8000;
	DebugLog(<<"Using input buffers of "<<bsize<<" bytes.");
	for(i = 0; i < WINSND_NBUFS; ++i)
	{
		WAVEHDR *hdr = &hdrs_read[i];
		add_input_buffer(hdr,bsize);
	}
	mr = waveInStart(indev);
	if (mr != MMSYSERR_NOERROR)
	{
		ErrLog(<<"waveInStart() error");
		return ;
	}
}

void WincardRead::card_read_postprocess()
{
	MMRESULT mr;
	int i;

	mr=waveInStop(indev);
	if (mr != MMSYSERR_NOERROR)
	{
		ErrLog(<<"waveInStop() error");
		return ;
	}
	mr=waveInReset(indev);
	if (mr != MMSYSERR_NOERROR)
	{
		ErrLog(<<"waveInReset() error");
		return ;
	}
	for(i=0;i<WINSND_NBUFS;++i)
	{
		WAVEHDR *hdr=&hdrs_read[i];

		if (hdr->dwUser == 0)
		{
			freemsg((mblk_t*)hdr->dwUser);
		}

		if (hdr->dwFlags & WHDR_PREPARED)
		{
			mr = waveInUnprepareHeader(indev,hdr,sizeof (*hdr));
			if (mr != MMSYSERR_NOERROR)
			{
				ErrLog(<<"waveInUnPrepareHeader() error");
			}
		}
	}
	mr = waveInClose(indev);
	if (mr != MMSYSERR_NOERROR)
	{
		ErrLog(<<"waveInClose() error");
		return ;
	}

	flushq(&rq,0);
}

void WincardRead::set_cardname(char* name)
{
	devname=name;
}

Data WincardRead::get_cardname()
{
	return devname;
}

void WincardRead::openMicBoost(bool boost)
{
	HMIXER                          hMixer; 
	MIXERLINE                       MixerLine; 
	MIXERCONTROL                    MixerControl; 
	MIXERLINECONTROLS               MixerLineControls; 
	PMIXERCONTROL                   paMixerControls; 
	MIXERCONTROLDETAILS             MixerControlDetails; 
	PMIXERCONTROLDETAILS_BOOLEAN    pMixerControlDetails_Boolean; 
	MIXERCAPS                       MixerCaps; 
	MMRESULT                        mmr; 

	UINT u,v, cConnections; 

	mmr = mixerOpen(&hMixer, dev_id, 0, 0L, MIXER_OBJECTF_MIXER); 

	if (mmr != MMSYSERR_NOERROR )
	{
		return;
	}

	mmr = mixerGetDevCaps((UINT)hMixer, &MixerCaps, sizeof(MixerCaps)); 

	for (u = 0; u < MixerCaps.cDestinations; u++) 
	{ 
		MixerLine.cbStruct      = sizeof(MixerLine); 
		MixerLine.dwDestination = u; 

		mmr = mixerGetLineInfo((HMIXEROBJ)hMixer, &MixerLine, MIXER_GETLINEINFOF_DESTINATION); 

		cConnections = (UINT)MixerLine.cConnections; 
		for (v = 0; v < cConnections; v++) 
		{ 
			MixerLine.cbStruct      = sizeof(MixerLine); 
			MixerLine.dwDestination = u; 
			MixerLine.dwSource      = v; 

			mmr = mixerGetLineInfo((HMIXEROBJ)hMixer, &MixerLine, MIXER_GETLINEINFOF_SOURCE); 

			if (MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == MixerLine.dwComponentType) 
				goto L_goon; 
		} 
	} 
L_goon: 
	paMixerControls = (PMIXERCONTROL)malloc(sizeof(MIXERCONTROL) * MixerLine.cControls); 

	MixerLineControls.cbStruct       = sizeof(MixerLineControls); 
	MixerLineControls.dwLineID       = MixerLine.dwLineID; 
	MixerLineControls.cControls      = MixerLine.cControls; 
	MixerLineControls.cbmxctrl       = sizeof(MIXERCONTROL); 
	MixerLineControls.pamxctrl       = paMixerControls; 

	mmr = mixerGetLineControls((HMIXEROBJ)hMixer, &MixerLineControls, MIXER_GETLINECONTROLSF_ALL);      

	for (u = 0; u < MixerLine.cControls; u++) 
		if (paMixerControls[u].dwControlType == MIXERCONTROL_CONTROLTYPE_ONOFF) 
			break; 

	MixerLineControls.cbStruct    = sizeof(MixerLineControls); 
	MixerLineControls.dwControlID = paMixerControls[u].dwControlID; 
	MixerLineControls.cbmxctrl    = sizeof(MixerControl); 
	MixerLineControls.pamxctrl    = &MixerControl; 

	mmr = mixerGetLineControls((HMIXEROBJ)hMixer, &MixerLineControls, MIXER_GETLINECONTROLSF_ONEBYID); 

	free(paMixerControls); 

	pMixerControlDetails_Boolean = (PMIXERCONTROLDETAILS_BOOLEAN)malloc(MixerLine.cChannels * sizeof(MIXERCONTROLDETAILS_BOOLEAN)); 

	MixerControlDetails.cbStruct       = sizeof(MixerControlDetails); 
	MixerControlDetails.dwControlID    = MixerControl.dwControlID; 
	MixerControlDetails.cChannels      = MixerLine.cChannels; 
	MixerControlDetails.cMultipleItems = MixerControl.cMultipleItems; 
	MixerControlDetails.cbDetails      = sizeof(sizeof(MIXERCONTROLDETAILS_BOOLEAN)); 
	MixerControlDetails.paDetails      = pMixerControlDetails_Boolean; 

	mmr = mixerGetControlDetails((HMIXEROBJ)hMixer, &MixerControlDetails, 0L); 

	// True to turn on boost, False to turn off 
	pMixerControlDetails_Boolean[0].fValue = boost;  

	mmr = mixerSetControlDetails((HMIXEROBJ)hMixer, &MixerControlDetails, 0L); 

	free(pMixerControlDetails_Boolean); 
}





