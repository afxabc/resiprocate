#include <iostream>
#include "audiorecord.h"
#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

//----------------------------------------------------------------------------
// fileplayer.cpp                                                                  
//----------------------------------------------------------------------------
using namespace std;
using namespace resip;
//## package Default 

//## class FilePlayer 
int emLen = 1;

AudioRecord::AudioRecord()
{
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.cbSize = 0;/*PCM格式时，忽略*/
	wfx.nAvgBytesPerSec = 16000;/*数据传输速率，字节/秒*/
	wfx.nBlockAlign = 2;/*每个样本的字节数*/
	wfx.nChannels = 1;/*通道数*/
	wfx.nSamplesPerSec = 8000;/*采样率*/
	wfx.wBitsPerSample = 16;/*每个样本的bit数*/

	qinit(&payload);

	recordOn = FALSE;
	nRecordBytes = 0;
	filename = "default.wav";//Data::Empty;
	//StartRecording(10,false);
}

AudioRecord::~AudioRecord() 
{
    flushq(&payload, 0);
}

bool AudioRecord::StartRecording(int nRecordVoice, bool bRecordCompress)
{
	//if nRecordVoice==0, record for ever
	nRecordBytes = wfx.nAvgBytesPerSec * nRecordVoice;
	recordOn = true;

	flushq(&payload, 0);

	return recordOn;
}

bool AudioRecord::StopRecording()
{
	if (false == recordOn)
	{
		return false;
	}
	recordOn = false;

	return filesave(filename);
	
}
bool AudioRecord::ResetRecording()
{
	recordOn = true;
	flushq(&payload, 0);

	return recordOn;	
}

bool AudioRecord::SaveRecordingToWaveFile(resip::Data name)
{
	if (Data::Empty == name)
	{
		name = "defaultRecord.wav";
	}

	filename = name;
    return true;
}

void AudioRecord::process(MediaContext &mc)
{
	mblk_t* inm = NULL;

	if (FALSE == recordOn)
	{
		return;
	}

	for (inm=qbegin(&mc.payload0); !qend(&mc.payload0,inm); inm=qnext(&mc.payload0,inm))
	{
        mblk_t *tmp;
		tmp = dupmsg(inm);
		putq(&payload, tmp);
	}

	//if the current len > nRecordBytes, then stop recording and savefile.
	int len = 0;
	for (inm=qbegin(&payload); !qend(&payload,inm); inm=qnext(&payload,inm))
	{
        len += msgdsize(inm);
	}

	if ((len >= nRecordBytes) && (nRecordBytes != 0))
	{
        StopRecording();
	}
	
/*	while(inm=getq(&mc.payload0))
	{
		putq(&payloadDat, inm);
	}*/	
}


bool AudioRecord::filesave(resip::Data name) 
{
	mblk_t *dat;
    fstream outfile;
	int len = 0;

	if (strstr(name.c_str(),".wav")==NULL)
	{
		ErrLog(<<"file is not wave");
		return false;
	}
	outfile.open(name.c_str(),ios::out|ios::binary);
	if(!outfile)
	{
		cout<<"open file failure"<<endl;
		return false;
	}

	riff_t riff;
	format_t format;
	data_t data;

	mblk_t* inm=NULL;
	for (inm=qbegin(&payload); !qend(&payload,inm); inm=qnext(&payload,inm))
	{
        len += msgdsize(inm);
	}

	memcpy(riff.riff,"RIFF",4);
	riff.len=len+36;
	memcpy(riff.wave,"WAVE",4);

	memcpy(format.fmt,"fmt ",4);
	format.rate = wfx.nSamplesPerSec;
	format.type = WAVE_FORMAT_PCM;
	format.bitpspl = wfx.wBitsPerSample;
	format.blockalign = wfx.nBlockAlign;
	format.channel = wfx.nChannels;
	format.bps = wfx.nAvgBytesPerSec;
	format.len = 16;

	memcpy(data.data,"data",4);
	data.len=len;
	
	outfile.write((char*)&riff,sizeof(riff_t));
	outfile.write((char*)&format,sizeof(format_t));
	outfile.write((char*)&data,sizeof(data_t));
	while (dat = getq(&payload))
	{
		while (dat)
		{
		    outfile.write((char*)dat->b_rptr, dat->b_wptr-dat->b_rptr);
			dat=dat->b_cont;
		}
		freemsg(dat);		
	}
	
	outfile.close();

	return true;
}

