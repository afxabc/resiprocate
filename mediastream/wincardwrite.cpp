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
#include "wincardwrite.h"
#include "rutil/Lock.hxx"
#include "mssndcard.h"
#include <MMREG.h>
#include <iostream>
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace std;
using namespace resip;


WincardWrite::WincardWrite(int devid)
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
	nbufs_playing = 0;

	confData = ms_bufferizer_new();
}

WincardWrite::WincardWrite(const WincardWrite& card):dev_id(card.dev_id)
{
	wfx.wFormatTag = card.wfx.wFormatTag;
	wfx.cbSize = card.wfx.cbSize;
	wfx.nAvgBytesPerSec = card.wfx.nAvgBytesPerSec;
	wfx.nBlockAlign = card.wfx.nBlockAlign;
	wfx.nChannels = card.wfx.nChannels;
	wfx.nSamplesPerSec = card.wfx.nSamplesPerSec;
	wfx.wBitsPerSample = card.wfx.wBitsPerSample;
}

WincardWrite::~WincardWrite()
{
	cardwrite_postprocess();

	ms_bufferizer_destroy(confData);
}

void WincardWrite::card_detect()
{
    MMRESULT mr = NOERROR;
    unsigned int nOutDevices=waveOutGetNumDevs ();
	unsigned int item;

	for(item = 0; item < nOutDevices; item++)
	{ 
        WAVEOUTCAPSA outcaps;

    	mr = waveOutGetDevCapsA (item, &outcaps, sizeof (WAVEOUTCAPSA));
        if (mr == MMSYSERR_NOERROR)
		{
//			parent->CardCreateWriter(item);
//			parent->SetPlayCardName(item,outcaps.szPname);
		}
    }
}

void WincardWrite::card_init()
{
	wfx.nBlockAlign = wfx.wBitsPerSample/8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec*wfx.nBlockAlign;
	outcurbuf = 0;
	nsamples = 0;
	
//	stat_minimumbuffer = WINSND_MINIMUMBUFFER;

	begin = false;
	bytes_read = 0;
}

MSBufferizer * WincardWrite::getConfBufptr()
{
	return confData;

}

unsigned long WincardWrite::getPitch()
{
	MMRESULT mr;
	unsigned long dwVolume;
	mr=waveOutGetPlaybackRate(outdev, &dwVolume);
	if (mr != MMSYSERR_NOERROR)
	{
		ErrLog(<<"Failed to get_volume_level ");
	}
	return dwVolume;
}
void WincardWrite::process(MediaContext &mc)
{
	unsigned char *old;
	mblk_t *dat=NULL;
	
	mblk_t* inm=NULL;
    flushq(&mc.payload1,0);
	for (inm=qbegin(&mc.payload0); !qend(&mc.payload0,inm); inm=qnext(&mc.payload0,inm))
	{
        mblk_t *tmp;
		tmp = dupmsg(inm);
		putq(&mc.payload1,tmp);
	}

	while(1)
	{
		int outcur = outcurbuf % WINSND_OUT_NBUFS;
		WAVEHDR *hdr = &hdrs_write[outcur];
		old = (unsigned char*)hdr->dwUser;
		if (nsamples < 500)
		{
			int tmpsize = WINSND_OUT_DELAY*wfx.nAvgBytesPerSec;
			mblk_t* tmpbuf = allocb(tmpsize,0);
			memset(tmpbuf->b_wptr,0,tmpsize);
			tmpbuf->b_wptr += tmpsize;
			playout_buf(hdr,tmpbuf);
			outcurbuf++;
			nsamples += WINSND_OUT_DELAY*wfx.nSamplesPerSec;
			continue;
		}
//		cout<<"payload0.q_mcount= "<<mc.payload0.q_mcount<<endl;;
		
		dat = getq(&mc.payload0);
		if(dat==NULL) break;

		/* volume control */
		int len = msgdsize(dat)/2;
		short *tmp = (short*)dat->b_rptr;
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

		nsamples += msgdsize(dat) / wfx.nBlockAlign;
//		mblk_t* buf = allocb(dat->size());
//		memcpy(buf->b_wptr,dat->c_str(),dat->size());
//		buf->b_wptr += dat->size();

		if( (wfx.wBitsPerSample == 16) && (wfx.nSamplesPerSec == 8000) )
		{
			int size = ms_bufferizer_get_avail(confData);
			int delayms = size * 1000 / wfx.nAvgBytesPerSec;
			int blen = msgdsize(dat);
			if( (delayms >= JITTER*2) && (size > blen) )
			{
				unsigned char drop[5120];
				ms_bufferizer_read(confData, drop, blen);
			}
			ms_bufferizer_put(confData, dupmsg(dat));
		}
		/*if the output buffer has finished to play, unprepare it*/
		if (hdr->dwFlags & WHDR_DONE)
		{
			if(waveOutUnprepareHeader(outdev,hdr,sizeof(*hdr)))
			{
				cout<<"waveOutUnprepareHeader error"<<endl;
			}
			freemsg((mblk_t*)old);
			old = NULL;
			hdr->dwFlags = 0;
			hdr->dwUser = 0;
		}
		if (old == NULL)
		{
			playout_buf(hdr,dat);
		}
		else
		{
			/* no more free wavheader, overrun !*/
			cout<<"WINSND overrun, restarting"<<endl;
			overrun = true;
			nsamples = 0;
			waveOutReset(outdev);
		}
		outcurbuf++;
	}
}

void CALLBACK
write_callback(HWAVEOUT outdev, UINT uMsg, DWORD dwInstance,
                 DWORD dwParam1, DWORD dwParam2)
{
	WAVEHDR *hdr=(WAVEHDR *) dwParam1;
	WincardWrite *d=(WincardWrite*)dwInstance;
	
	switch (uMsg){
		case WOM_OPEN:
			break;
		case WOM_CLOSE:
		case WOM_DONE:
			if (hdr)
			{
				d->nbufs_playing--;
			}
			break;
	}
}

void WincardWrite::cardwrite_preprocess()
{
	DWORD dwFlag;
	MMRESULT mr;
	int i;

	wfx.nBlockAlign = wfx.nChannels*wfx.wBitsPerSample / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	/* Init Microphone device */
	dwFlag = CALLBACK_FUNCTION;
	if (dev_id != WAVE_MAPPER)
	{
		dwFlag = WAVE_MAPPED | CALLBACK_FUNCTION;
	}
	mr = waveOutOpen (&outdev, dev_id, &wfx,(DWORD)write_callback, (DWORD)this, dwFlag);
	if (mr != MMSYSERR_NOERROR)
	{
		ErrLog(<<"Failed to open windows sound device :"<<dev_id);
		mr = waveOutOpen (&outdev, WAVE_MAPPER, &wfx,(DWORD)write_callback, (DWORD)this, CALLBACK_FUNCTION);
		if (mr != MMSYSERR_NOERROR)
		{
			ErrLog(<<"Failed to open windows sound device :"<<dev_id);
			outdev=NULL;
			return ;
		}
	}
	/*
	dwFlag = CALLBACK_FUNCTION;
	if(waveOutOpen(NULL,dev_id,&wfx,NULL,NULL,WAVE_FORMAT_QUERY))
	{
		cout<<"check error"<<endl;
		return;
	}
	if(waveOutOpen(&outdev,dev_id,&wfx,(DWORD)write_callback, 0, dwFlag))
	{
		cout<<"waveopen error"<<endl;
		return;
	}
	*/
	for(i = 0; i < WINSND_OUT_NBUFS; ++i)
	{
		WAVEHDR *hdr = &hdrs_write[i];
		hdr->dwFlags = 0;
		hdr->dwUser = 0;
	}
	outcurbuf = 0;
	overrun = false;
}

void WincardWrite::cardwrite_postprocess()
{
	MMRESULT mr;
	int i;
	if (outdev==NULL) return;
	mr=waveOutReset(outdev);
	if (mr != MMSYSERR_NOERROR)
	{
		ErrLog(<<"waveOutReset() error");
		return ;
	}
	for(i=0;i<WINSND_OUT_NBUFS;++i)
	{
		WAVEHDR *hdr=&hdrs_write[i];
		mblk_t *old;
		if (hdr->dwFlags & WHDR_DONE)
		{
			mr=waveOutUnprepareHeader(outdev,hdr,sizeof(*hdr));
			if (mr != MMSYSERR_NOERROR)
			{
				ErrLog(<<"waveOutUnprepareHeader error");
			}
			old=(mblk_t*)hdr->dwUser;
			if (old) 
			{
				freemsg(old);
			}
			hdr->dwUser=0;
		}
	}
	mr=waveOutClose(outdev);
	if (mr != MMSYSERR_NOERROR)
	{
		ErrLog(<<"waveOutClose() error");
		return ;
	}
//	ready=0;
//	workaround=0;
}

void WincardWrite::playout_buf(WAVEHDR *hdr, mblk_t* m)
{
	MMRESULT mr;
	hdr->dwUser = (DWORD)m;
	hdr->lpData = (LPSTR)m->b_rptr;
	hdr->dwBufferLength = msgdsize(m);
	hdr->dwFlags = 0;
	mr = waveOutPrepareHeader(outdev,hdr,sizeof(*hdr));
	if (mr != MMSYSERR_NOERROR)
	{
		cout<<"waveOutUnprepareHeader error"<<endl;
	}
	mr = waveOutWrite(outdev,hdr,sizeof(*hdr));
	if (mr != MMSYSERR_NOERROR)
	{
		cout<<"waveOutWrite() error"<<endl;
	}
	else
	{
		nbufs_playing++;
	}
}

void WincardWrite::setFormat(WORD format)
{
	wfx.wFormatTag = format;
	
}

void WincardWrite::setAvgBytesPerSec(DWORD bytes)
{
	wfx.nAvgBytesPerSec = bytes;/*数据传输速率，字节/秒*/
	
}

void WincardWrite::setBlockAlign(WORD aglin)
{
	wfx.nBlockAlign = aglin;/*每个样本的字节数*/

}

void WincardWrite::setChannels(WORD chnnl)
{
	wfx.nChannels = chnnl;/*通道数*/

}

void WincardWrite::setSamples(DWORD sample)
{
	wfx.nSamplesPerSec = sample;/*采样率*/

}

void WincardWrite::setBitsPerSample(WORD bits)
{
	wfx.wBitsPerSample = bits;
}


void WincardWrite::set_cardname(char* name)
{
	devname=name;
}

Data WincardWrite::get_cardname()
{
	return devname;
}

