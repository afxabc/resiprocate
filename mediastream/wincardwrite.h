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
#ifndef WinscardWrite_H
#define WinscardWrite_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include "str_utils.h"
#include <mmsystem.h>
#include "rutil/Mutex.hxx"
#include "rutil/Data.hxx"
#include "MediaContext.h"
#include "processor.h"


void CALLBACK write_callback(HWAVEOUT outdev, UINT uMsg, DWORD dwInstance,
                 DWORD dwParam1, DWORD dwParam2);

class MSSndCard;
using namespace resip;

class WincardWrite: public Processor
{
public:
	WincardWrite(int devid);
	WincardWrite(const WincardWrite& card);
	virtual ~WincardWrite();

	static void card_detect();

	void card_init();

	int get_dev_id() {return dev_id; };
	void set_volume_level(float i) {level = i;}
	float get_volume_level() {return level;}
	void set_mute() {mute = 0;}
	void clear_mute() {mute = 1;}
	unsigned int get_peak() {return peak;}


	virtual void process(MediaContext &mc);

	void playout_buf(WAVEHDR *hdr, mblk_t* m);
	void cardwrite_preprocess();
	void cardwrite_postprocess();
	void setFormat(WORD format);
	void setAvgBytesPerSec(DWORD bytes);
	void setBlockAlign(WORD aglin);
	void setChannels(WORD chnnl);
	void setSamples(DWORD sample);
	void setBitsPerSample(WORD bits);

	MSBufferizer * getConfBufptr();

	void set_cardname(char* name);
	unsigned long getPitch();
	resip::Data get_cardname();
	bool isPlayComplete() {return nbufs_playing==0;};
	int getPID() const {return pid; };
	void setPID(int id) {pid = id; };

public:
	
	int nbufs_playing;

private:
	resip::Data devname;
	int dev_id;
//	HWAVEIN indev;
	HWAVEOUT outdev;
	WAVEFORMATEX wfx;

	WAVEHDR hdrs_write[WINSND_OUT_NBUFS];

	int outcurbuf;
	int nsamples;
	bool begin;
	int bytes_read;
	MSBufferizer *confData;

	__int32 stat_minimumbuffer;
	bool overrun;

	MSSndCard *mCardManager;
	int pid;

	float level;           /*range: 0 - 1*/
	short mute;            /* 0:mute, 1:not mute */
	unsigned int peak;
};
#endif
