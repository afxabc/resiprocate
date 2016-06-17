#include <iostream>
#include "audioplayer.h"
#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"
#include "wincardwrite.h"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

//----------------------------------------------------------------------------
// audioplayer.cpp                                                                  
//----------------------------------------------------------------------------
using namespace std;
using namespace resip;


AudioPlayer::AudioPlayer(int val)
{
	codectype=0;
	nAvgBytesPerSec = 16000;      /*bytes per second*/
	wBitsPerSample = 16;          /*bits per sample*/
	nSamplesPerSec = 8000;        /*sample rate*/
	nchannels = 1;                /*channel*/

	interval=val;
	hsize=0;
	filelen=0;	
	rb=NULL;

	localPlayOn = FALSE;
	playOn = FALSE;
	localCardwrite = NULL;
	context = NULL;	

	//qinit(&payloadDat);
}

AudioPlayer::~AudioPlayer() 
{
	if (rb)
	{
		freemsg(rb);
	}
	if (localCardwrite)
	{
		delete localCardwrite;
	}
	if (context)
	{
		delete context;
	}

	//flushq(&payloadDat, 0);
}

bool AudioPlayer::PlayWaveOpen(Data sFileName)
{
//	Lock lock(mMutex); (void)lock;
	if(rb)
	{
		freemsg(rb);
		rb = NULL;
	}
	if (Data::Empty == sFileName)
	{
		sFileName = "N:/resiprocate-1.4/apps/example.wav";
	}
	return fileopen(sFileName); 
}

bool AudioPlayer::PlayWaveSkipTo(int nSeconds)
{
	if (!playOn)
		return false;
	/*320bytes=20ms, 16000byes=1s*/
	hsize += nAvgBytesPerSec * nSeconds;
	if (hsize > filelen)
	{
        hsize = filelen;
		return false;
	}
	else if (hsize < 0)
	{
		hsize = 0;
		return false;
	}

	return true;
}

int AudioPlayer::PlayWaveTotalTime()
{
	return filelen / nAvgBytesPerSec;

}

bool AudioPlayer::PlayWavePause()
{
	playOn = false;

	return true;
}

bool AudioPlayer::PlayWaveStart(bool bListen)
{
//	Lock lock(mMutex); (void)lock;

	playOn = true;

	return LocalPlay(bListen);

}

bool AudioPlayer::PlayWaveStop()
{
//	Lock lock(mMutex); (void)lock;

	if (false == playOn)
	{
		return false;
	}
	playOn = false;
	hsize = 0;

    if(rb)
	{
		freemsg(rb);
		rb = NULL;
	}

	LocalPlay(false);
	return true;
}

int AudioPlayer::PlayWavePosition()
{
	return hsize / nAvgBytesPerSec;
}


bool AudioPlayer::LocalPlay(bool stat)
{

	localPlayOn = stat;
	if (localCardwrite)
	{
		delete localCardwrite;
		localCardwrite = NULL;
	}
	if (context)
	{
		delete context;
		context = NULL;
	}

	if (stat)
	{
		localCardwrite = new WincardWrite(0);
		WincardWrite::card_detect();
		localCardwrite->card_init();
		localCardwrite->cardwrite_preprocess();
		context = new MediaContext();
	}

	return stat;
}

void AudioPlayer::process(MediaContext &mc)
{
	mblk_t* inm;

	if (!playOn)
		return;

	Lock lock(mMutex); (void)lock;

	flushq(&mc.payload0, 0);

	int bytes = wBitsPerSample/8 * nSamplesPerSec * nchannels * interval/1000;
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

  		    //local playing
			if (localPlayOn)
			{
			    mblk_t* tmp;
			    tmp = dupb(inm);
			    putq(&context->payload0,tmp);
			    localCardwrite->process(*context);
			}
		}
		else
		{
//			data.empty();
		}
		
		if (err<bytes)
		{
			//?????
            playOn = false;
			hsize=0;
			InfoLog(<<"read over and stop playing");
		
		}
	}
}

bool AudioPlayer::fileopen(Data name) 
{
	if (strstr(name.c_str(),".wav")==NULL)
	{
		ErrLog(<<"file is not wave");
		return false;
	}
	file.open(name.c_str(),ios::in|ios::binary);
	if(!file)
	{
		ErrLog(<<"Failed to open:");
		return false;
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
		return false;
	}

	file.read(header2, sizeof(header2));  
	//codectype=le_uint16(format_chunk->type);
	int lSamplesPerSec=le_uint32(format_chunk->rate);
	int lchannels=le_uint16(format_chunk->channel);
	//codectype=le_uint32(format_chunk->type);
	//nAvgBytesPerSec = le_uint32(format_chunk->bps);
	//wBitsPerSample=le_uint32(format_chunk->bitpspl);
	
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
    
	mblk_t *rbOld;
	rbOld = allocb(filelen, 0);
	file.read((char*)rbOld->b_rptr,filelen);
	//rbOld->b_wptr += filelen;
	file.close();


	int i;
	/*channels convert*/
	if (lchannels == 2)
	{
		filelen = filelen / 2;
		for (i=0; i<filelen/2; i++)
		{
			*(unsigned short*)&(rbOld->b_rptr[i*2]) = *(unsigned short*)&(rbOld->b_rptr[i*2*2]);
		}

	}

	rbOld->b_wptr += filelen;

	/*Samples rate convert*/
	int k = lSamplesPerSec/8000;
	if(k<0) k=1;
	filelen = filelen / k;
	rb=allocb(filelen,0);
	for (i=0; i<filelen/2; i++)
	{
		*(unsigned short*)&(rb->b_rptr[i*2]) = *(unsigned short*)&(rbOld->b_rptr[i*k*2]);
	}
	rb->b_wptr += filelen;

	/*for test*/
	/*putq(&payloadDat, rb);
	resip::Data name1 = "out.wav";
	filesave(name1);*/

	hsize = 0;

	freemsg(rbOld);
	
	InfoLog(<<"opened: rate="<<nSamplesPerSec<<"channel="<<nchannels);
	return true;
}

int AudioPlayer::Convert44kTo8k(unsigned short*in, int shLen, unsigned char*out)
{
	int outLen = 0;
	int mDataSize = shLen;
	int MAX_SIZE = 80 * 441;
	unsigned short data44k[441];
	unsigned short data[80*441];
	int i,m,k,j;

    while(mDataSize >= 441)                                             
   {  
	  memcpy((unsigned char *)data44k, (unsigned char*)in, 441*2);
  
      mDataSize = mDataSize - 441;   
    
      for(i=0;   i<MAX_SIZE;   i+=80)   
      {   
         data[i] = data44k[i/80];   
      }   
    
      for(m=0;m<MAX_SIZE;m+=80)   
      {   
         k=(data[m+80]-data[m])/80;   
         for(j=0;j<80;j++)   
         {   
             data[m+j]=data[m]+k*j;     
         }   
      }   
    
      for(i=0;   i<MAX_SIZE;   i+=441)   
      {   
          data44k[i/441] = data[i];   
      }   
    
	  memcpy(&out[outLen], (unsigned char*)data44k, 80*2);
      
      outLen += 2*80;   
  }  

	return outLen;

}

/*
int AudioPlayer::filesave(resip::Data name) 
{
	mblk_t *dat;
    fstream outfile;
	int len = 0;

	if (strstr(name.c_str(),".wav")==NULL)
	{
		ErrLog(<<"file is not wave");
		return -1;
	}
	outfile.open(name.c_str(),ios::out|ios::binary);
	if(!outfile)
	{
		cout<<"open file failure"<<endl;
		return 0;
	}

	riff_t riff;
	format_t format;
	data_t data;

	mblk_t* inm=NULL;
	for (inm=qbegin(&payloadDat); !qend(&payloadDat,inm); inm=qnext(&payloadDat,inm))
	{
        len += msgdsize(inm);
	}

	memcpy(riff.riff,"RIFF",4);
	riff.len=len+36;
	memcpy(riff.wave,"WAVE",4);

	memcpy(format.fmt,"fmt ",4);
	format.rate = 8000;
	format.type = 1;
	format.bitpspl = 16;
	format.blockalign = 2;
	format.channel = 1;
	format.bps = 16000;
	format.len = 16;

	memcpy(data.data,"data",4);
	data.len=len;
	
	outfile.write((char*)&riff,sizeof(riff_t));
	outfile.write((char*)&format,sizeof(format_t));
	outfile.write((char*)&data,sizeof(data_t));
	while (dat = getq(&payloadDat))
	{
		while (dat)
		{
		    outfile.write((char*)dat->b_rptr, dat->b_wptr-dat->b_rptr);
			dat=dat->b_cont;
		}
		freemsg(dat);		
	}
	
	outfile.close();

	return 0;
}
*/
