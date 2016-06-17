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
#include <iostream>
#include "rutil/Lock.hxx"
#include "rutil/Logger.hxx"
#include "videofile.h"
#include "msvideo.h"
#define RESIPROCATE_SUBSYSTEM Subsystem::APP
using namespace std;
using namespace resip;

Videofile::Videofile(MSTicker *tick)
{
	file.open("videofile304128", ios::in|ios::binary);
	ticker=tick;
	total=0;
}

Videofile::~Videofile()
{
	file.close();
}

void Videofile::process(MediaContext& mc)
{
	mblk_t *m;
	unsigned __int32 timestamp;
	int cur_frame;
	MSVideoSize roi;
	roi.width=352;
	roi.height=288;

	if(mc.payload0.q_mcount>0)
	{
		flushq(&mc.payload0,0);
		InfoLog(<<"Winvideo flushq");
	}

	m = (mblk_t*)allocb(304128,0);

	file.read((char*)m->b_wptr, 304128); 
	m->b_wptr += 304128;
	
	total+=304128;
	if(total >= 72686592)
	{
		file.seekg(0,ios::beg);
		total=0;
	}
	rgb24_revert(m->b_rptr,roi.width,roi.height,roi.width*3);
	timestamp=ticker->time*90;/* rtp uses a 90000 Hz clockrate for video*/
	mblk_set_timestamp_info(m,timestamp);
	putq(&mc.payload0,m);

}

void Videofile::rgb24_revert(uint8_t *buf, int w, int h, int linesize)
{
	uint8_t *p,*pe;
	int i,j;
	uint8_t *end=buf+(h*linesize);
	uint8_t exch;
	p=buf;
	pe=end-1;
	for(i=0;i<h/2;++i){
		for(j=0;j<w*3;++j){
			exch=p[j];
			p[j]=pe[-j];
			pe[-j]=exch;
		}
		p+=linesize;
		pe-=linesize;
	}	
}