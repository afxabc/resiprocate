/********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: fileplayer
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\fileplayer.cpp
*********************************************************************/
#include <iostream>
#include "fileplayer.h"
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


FilePlayer::FilePlayer(int val)
{
	codectype=0;
	byteps=0;/*byte/s*/
	bitpersample=0;/*byte per sample*/
	rate=0;/*sample rate*/
	nchannels=0;/*channel*/
	hsize=0;
	filelen=0;
	loop_after=0;
	pause_time=0;
	interval=val;
	rb=NULL;
}

FilePlayer::~FilePlayer() 
{
	if(rb!=NULL)
	{
		freemsg(rb);
	}
}

void FilePlayer::process(MediaContext &mc)
{
	mblk_t* inm;

	int bytes=bitpersample/8*interval*rate*nchannels/1000;
	int err=MIN(filelen-hsize,bytes);
	
	if (err>=0)
	{
		if (err!=0)
		{
			inm=allocb(err,0);
			memcpy(inm->b_rptr,rb->b_rptr+hsize,err);
			inm->b_wptr+=err;
			hsize+=err;
			putq(&mc.payload0,inm);
//			context.SetPayloadType(0xff);
		}
		else
		{
//			data.empty();
		}
		
		if (err<bytes)
		{
			//ms_filter_notify_no_arg(f,MS_FILE_PLAYER_EOF);
			hsize=0;
			InfoLog(<<"read over and read again");
		}
	}
}

int FilePlayer::fileopen(Data name) 
{
    //#[ operation initial(msfilter) 
    //#]
	if (strstr(name.c_str(),".wav")==NULL)
	{
		ErrLog(<<"file is not wave");
		return -1;
	}
	file.open(name.c_str(),ios::in|ios::binary);
	if(!file)
	{
		ErrLog(<<"Failed to open:");
		return -1;
	}
	char header1[sizeof(riff_t)];
	char header2[sizeof(format_t)];
	char header3[sizeof(data_t)];
	int count;

	riff_t *riff_chunk=(riff_t*)header1;
	format_t *format_chunk=(format_t*)header2;
	data_t *data_chunk=(data_t*)header3;

	unsigned long len=0;

	file.read(header1,sizeof(header1));
	
	if (0!=strncmp(riff_chunk->riff, "RIFF", 4) || \
		0!=strncmp(riff_chunk->wave, "WAVE", 4))
	{	
		ErrLog(<<"Wrong wav header (not RIFF/WAV)");
		return -1;
	}

	file.read(header2, sizeof(header2)); 
	codectype=le_uint16(format_chunk->type);
	rate=le_uint32(format_chunk->rate);
	nchannels=le_uint16(format_chunk->channel);
	codectype=le_uint32(format_chunk->type);
	byteps=le_uint32(format_chunk->bps);
	bitpersample=le_uint32(format_chunk->bitpspl);
	
	if (format_chunk->len-0x10 > 0)
	{
		file.seekg(format_chunk->len-0x10,ios::cur);
	}

	hsize=sizeof(wave_header_t)-0x10+format_chunk->len;

	file.read(header3, sizeof(header3));
	
	count=0;
	while (strncmp(data_chunk->data, "data", 4)!=0 && count<30)
	{
		ErrLog(<<"skipping chunk="<<data_chunk->data<<"len="<<data_chunk->len);
		file.seekg(data_chunk->len,ios::cur);
		count++;
		hsize=hsize+sizeof(header3)+data_chunk->len;

		file.read(header3, sizeof(header3));
	}
	filelen = data_chunk->len;

	rb=allocb(filelen,0);
	file.read((char*)rb->b_rptr,filelen);
	rb->b_wptr+=filelen;
	hsize = 0;
	file.close();
	InfoLog(<<"opened: rate="<<rate<<"channel="<<nchannels);
	return 0;
}


/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\fileplayer.cpp
*********************************************************************/

