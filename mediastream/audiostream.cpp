/********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: audiostream
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\audiostream.cpp
*********************************************************************/
#include <iostream>
#include <time.h>
#include "audiostream.h"
#include "audiodecode.h"
#include "audioencode.h"
#include "fileplayer.h"
#include "processor.h"
#include "processorchain.h"
#include "rtpread.h"
#include "rtpwrite.h"
#include "wincardread.h"
#include "wincardwrite.h"
#include "audioplayer.h"
#include "audiorecord.h"
#include "speexec.h"
#include "dtmf.h"
#include "gtagc.h"
#include "MediaContext.h"
#include "audioconf.h"
//#include "apps/AppSession.hxx"
//#include "apps/DialInstance.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

//----------------------------------------------------------------------------
// audiostream.cpp                                                                  
//----------------------------------------------------------------------------
using namespace std;
using namespace resip;
//## package Default 

//## class audiostream 
static int dtmf_tab[16]={'0','1','2','3','4','5','6','7','8','9','*','#','A','B','C','D'};

//void on_dtmf_received(RTPSession *s, int dtmf, void * user_data)
//{
//	msfilter *dtmfgen=(msfilter*)user_data;
//	if (dtmf>15){
//		return;
//	}
//	ms_message("Receiving dtmf %c.",dtmf_tab[dtmf]);
//	if (dtmfgen!=NULL){
//		ms_filter_call_method(dtmfgen,MS_DTMF_GEN_PUT,&dtmf_tab[dtmf]);
//	}
//}

//void payload_type_changed(RtpSession *session, unsigned long data){
//	audiostream *stream=(audiostream*)data;
//	int pt=rtp_session_get_recv_payload_type(stream->session);
//	audio_stream_change_decoder(stream,pt);
//}

AudioStream::AudioStream():
	session(NULL),
	cardread(NULL),
	encode(NULL),
	rtpwrite(NULL),
	rtpread(NULL),
	decode(NULL),
	cardwrite(NULL),
	fileread(NULL),
	fileplayer(NULL),
	filerecorder(NULL),
	speexec(NULL),
	dtmf(NULL),
	agc(NULL),
	audioprocess(NULL),
	audioring(NULL),
	localring(NULL),
	conf(NULL)
{
	runflag=false;
	talking=false;
}

AudioStream::~AudioStream()
{
	InfoLog(<<"delete AudioStream ");
}

RTPSession *AudioStream::create_rtpsession( int locport)
{
	session = new RTPSession();
	int status = session->Create(locport);
	if(status < 0)
	{
		return NULL;
	}

	return session;
}

int AudioStream::audiostream_start(const char *remip,
								   int remport,
								   unsigned char payload,
								   int capId,
								   int playId,
								   bool ec,
								   int gc,
								   bool boost)
{
	
	InfoLog(<<"remoteip="<<remip<<"remoteport="<<remport<<"media type="<<payload);
	unsigned long destip = inet_addr(remip);
	if (destip == INADDR_NONE)
	{
		ErrLog(<<"Bad IP address specified");
		return -1;
	}
	destip = ntohl(destip);
	session->AddDestination(destip, remport);

	audioprocess = new ProcessorChain();

	cardread = new WincardRead(capId);
	WincardRead::card_detect();
	cardread->card_init();
	cardread->card_read_preprocess();
	this->SetMicVolume(this->miclevel);
	this->SetMicMute(this->micmute);
	if(boost)
	{
		this->SetMicBoost(boost);
	}
	cardread->setAudioConf(this->conf);
	cardread->setPID((int)this);
	audioprocess->addProcessor(std::auto_ptr<Processor>(cardread));
	
	speexec = new SpeexEc();
	audioprocess->addProcessor(std::auto_ptr<Processor>(speexec));
	if(ec)
	{
		speexec->OpenEchoCancellation();
	}
	fileplayer=new AudioPlayer();
	audioprocess->addProcessor(std::auto_ptr<Processor>(fileplayer));

	encode = new AudioEncode(payload);
	audioprocess->addProcessor(std::auto_ptr<Processor>(encode));
	
	rtpwrite = new RtpWrite(session);
	audioprocess->addProcessor(std::auto_ptr<Processor>(rtpwrite));

	rtpread = new RtpRead(session);
	audioprocess->addProcessor(std::auto_ptr<Processor>(rtpread));
	
	dtmf = new Dtmf();
	audioprocess->addProcessor(std::auto_ptr<Processor>(dtmf));

	decode = new AudioDecode();
	audioprocess->addProcessor(std::auto_ptr<Processor>(decode));
	
	filerecorder=new AudioRecord();
	audioprocess->addProcessor(std::auto_ptr<Processor>(filerecorder));

	agc=new Agc();
	audioprocess->addProcessor(std::auto_ptr<Processor>(agc));
	if(gc>0)
	{
		agc->OpenAgc();
	}

	cardwrite = new WincardWrite(playId);
	WincardWrite::card_detect();
	cardwrite->card_init();
	cardwrite->cardwrite_preprocess();

	if(conf)
	{
		conf->addMember(cardwrite);
	}
	cardwrite->setPID((int)this);

	this->SetSpkVolume(this->spklevel);
	this->SetSpkMute(this->spkmute);
	audioprocess->addProcessor(std::auto_ptr<Processor>(cardwrite));

	MSTicker *tick = getTicker();//app->getParent()->getAudioTicker();
	Lock *lock = new Lock(tick->mutex);
	tick->executechain.addProcessor(std::auto_ptr<Processor>(audioprocess));
	delete lock;
	tick->restart();

	runflag=true;
	talking=true;

	return 0;
}

int AudioStream::receive_mediastream(unsigned char payload,int playId)
{
	audioring = new ProcessorChain();

	rtpread = new RtpRead(session);
	audioring->addProcessor(std::auto_ptr<Processor>(rtpread));
	
	decode = new AudioDecode();
	audioring->addProcessor(std::auto_ptr<Processor>(decode));

	cardwrite = new WincardWrite(playId);
	WincardWrite::card_detect();
	cardwrite->card_init();
	cardwrite->cardwrite_preprocess();
//	this->SetSpkVolume(this->spklevel);
//	this->SetSpkMute(this->spkmute);
	audioring->addProcessor(std::auto_ptr<Processor>(cardwrite));

	MSTicker *tick = getTicker();//app->getParent()->getAudioTicker();
	Lock *lock = new Lock(tick->mutex);
	tick->executechain.addProcessor(std::auto_ptr<Processor>(audioring));
	delete lock;
	tick->restart();

	runflag=true;

	return 0;
}

void AudioStream::winsndcardDect(string& outname, string& inname)
{
//	WincardWrite* cardwrite = new WincardWrite();
//	outname=cardwrite->card_detect(0);

//	WincardRead* cardread = new WincardRead();
//	inname=cardread->card_detect();

//	delete cardread;
//	delete cardwrite;
}

void AudioStream::audiostream_stop()
{
	MSTicker *tick = getTicker();//app->getParent()->getAudioTicker();
	if(audioprocess)
	{
		if(conf) conf->remMember(cardwrite);

		Lock *lock = new Lock(tick->mutex);
		tick->executechain.remProcessor(audioprocess);
		delete lock;
		audioprocess = NULL;
	}
	if(audioring)
	{
		Lock *lock = new Lock(tick->mutex);
		tick->executechain.remProcessor(audioring);
		delete lock;
		audioring = NULL;
	}
	if(localring)
	{
		Lock *lock = new Lock(tick->mutex);
		tick->executechain.remProcessor(localring);
		delete lock;
		localring = NULL;
	}
	if(tick->executechain.getChainSize() == 0)
	{
		tick->shutdown();
		tick->join();

		InfoLog(<<"shutdown tick ");
	}
	if(session!=NULL)
	{
		InfoLog(<<"delete session");
		delete session;
		session=NULL;
	}
	cardread=NULL;
	encode=NULL;
	rtpwrite=NULL;
	rtpread=NULL;
	decode=NULL;
	cardwrite=NULL;
	fileplayer=NULL;
	filerecorder=NULL;
	dtmf=NULL;
	agc=NULL;

	runflag=false;
	talking=false;
}

bool AudioStream::audiostream_alive(int timeout)
{
//	const rtp_stats_t *stats = rtp_session_get_stats(session);
//	if (stats->recv !=0 ){
//		if (stats->recv != last_packet_count){
//			last_packet_count = stats->recv;
//			last_packet_time = time(NULL);
//		}else{
//			if (time(NULL) - last_packet_time > timeout){
				/* more than timeout seconds of inactivity*/
//				return false;
//			}
//		}
//	}
	return true;
}

void AudioStream::ringstream_start(const char* file,int playId)
{
	
	localring = new ProcessorChain();
	
	fileread = new FilePlayer(20);
	int result=fileread->fileopen(file);
	if(result>=0)
	{
		localring->addProcessor(std::auto_ptr<Processor>(fileread));

		cardwrite = new WincardWrite(playId);
		cardwrite->setFormat(fileread->codectype);
		cardwrite->setAvgBytesPerSec(fileread->byteps);
		cardwrite->setBlockAlign(fileread->bitpersample/8);
		cardwrite->setChannels(fileread->nchannels);
		cardwrite->setSamples(fileread->rate);
		cardwrite->setBitsPerSample(fileread->bitpersample);

		WincardWrite::card_detect();
		cardwrite->card_init();
		cardwrite->cardwrite_preprocess();
//		this->SetSpkVolume(this->spklevel);
//		this->SetSpkMute(this->spkmute);
		localring->addProcessor(std::auto_ptr<Processor>(cardwrite));
	}
	MSTicker *tick = getTicker();//app->getParent()->getAudioTicker();
	Lock *lock = new Lock(tick->mutex);
	tick->executechain.addProcessor(std::auto_ptr<Processor>(localring));
	delete lock;
	tick->restart();

	runflag=true;
}

void AudioStream::ringstream_stop()
{
	MSTicker *tick = getTicker();//app->getParent()->getAudioTicker();
	if(localring)
	{
		Lock *lock = new Lock(tick->mutex);
		tick->executechain.remProcessor(localring);
		delete lock;
		localring = NULL;
	}
	if(tick->executechain.getChainSize() == 0)
	{
		tick->shutdown();
		tick->join();

		InfoLog(<<"shutdown tick ");
	}
	cardwrite=NULL;
	fileread=NULL;

	runflag=false;
}


bool AudioStream::IsRecording()
{
	if(filerecorder)
	{
		return filerecorder->recordOn;
	}
	return false;
}

bool AudioStream::StartRecording(int nRecordVoice, bool bRecordCompress)
{
	if(filerecorder)
	{
		return filerecorder->StartRecording(nRecordVoice, bRecordCompress);
	}
	return false;
}

bool AudioStream::StopRecording()
{
	if(filerecorder)
	{
		return filerecorder->StopRecording();
	}
	return false;
}

bool AudioStream::ResetRecording()
{
	if(filerecorder)
	{
		return filerecorder->ResetRecording();
	}
	return false;
}

bool AudioStream::SaveRecordingToWaveFile(Data sFileName)
{
	if(filerecorder)
	{
		return filerecorder->SaveRecordingToWaveFile(sFileName);
	}
	return false;
}

bool AudioStream::IsWaveFilePlaying()
{
	if(fileplayer)
	{
		return fileplayer->playOn;
	}
	return false;
}

bool AudioStream::PlayWaveOpen(Data sFileName)
{
	if(fileplayer)
	{
		return fileplayer->PlayWaveOpen(sFileName);
	}
	return false;
}

bool AudioStream::PlayWaveSkipTo(int nSeconds)
{
	if(fileplayer)
	{
		return fileplayer->PlayWaveSkipTo(nSeconds);
	}
	return false;
}

int AudioStream::PlayWaveTotalTime()
{
	if(fileplayer)
	{
		return fileplayer->PlayWaveTotalTime();
	}
	return 0;
}

bool AudioStream::PlayWavePause()
{
	if(fileplayer)
	{
		return fileplayer->PlayWavePause();
	}
	return false;
}

bool AudioStream::PlayWaveStart(bool bListen)
{
	if(fileplayer)
	{
		return fileplayer->PlayWaveStart(bListen);
	}
	return false;
}

bool AudioStream::PlayWaveStop()
{
	if(fileplayer)
	{
		return fileplayer->PlayWaveStop();
	}
	return false;
}

int AudioStream::PlayWavePosition()
{
	if(fileplayer)
	{
		return fileplayer->PlayWavePosition();
	}
	return 0;
}

int AudioStream::GetMicSoundLevel()
{
	if(cardread)
	{
		return cardread->get_peak();
	}
	return 0;
}

int AudioStream::GetSpkSoundLevel()
{
	if(cardwrite)
	{
		return cardwrite->get_peak();
	}
	return 0;
}

int AudioStream::GetMicVolume()
{
	if(cardread)
	{
		float level=cardread->get_volume_level();
		return level*0x7fff;
	}
	return 0;
}

bool AudioStream::SetMicVolume(int nVolume)
{
	if(cardread)
	{
		float level=(float)nVolume/0xdfff;
		cardread->set_volume_level(level);
	}
	return true;
}

int AudioStream::GetSpkVolume()
{
	if(cardwrite)
	{
		float level=cardwrite->get_volume_level();
		return level*0x7fff;
	}
	return 0;
}

bool AudioStream::SetSpkVolume(int nVolume)
{
	if(cardwrite)
	{
		float level=(float)nVolume/0xdfff;
		cardwrite->set_volume_level(level);
	}
	return true;
}

void AudioStream::SetMicMute(bool bMute)
{
	if(cardread)
	{
		if(bMute)
		{
			cardread->set_mute();
		}
		else
		{
			cardread->clear_mute();
		}
	}
}

void AudioStream::SetSpkMute(bool bMute)
{
	if(cardwrite)
	{
		if(bMute)
		{
			cardwrite->set_mute();
		}
		else
		{
			cardwrite->clear_mute();
		}
	}
}

void AudioStream::SetEchoCancel(bool ec)
{
	if(speexec)
	{
		if(ec)
		{
			speexec->OpenEchoCancellation();
		}
		else
		{
			speexec->CloseEchoCancellation();
		}
	}
}

void AudioStream::SetAGC(int gc)
{
	if(agc)
	{
		if(gc>0)
		{
			agc->OpenAgc();
		}
		else
		{
			agc->CloseAgc();
		}
	}
}

void AudioStream::SetMicBoost(bool boost)
{
	if(cardread)
	{
		if(boost)
		{
			cardread->openMicBoost(true);
		}
		else
		{
			cardread->openMicBoost(false);
		}
	}
}

void AudioStream::SendDTMF(Data digit)
{
	if((dtmf!=NULL)&&(session!=NULL))
	{
		dtmf->Dtmf_send(session,*(digit.data()));
	}
}
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\audiostream.cpp
*********************************************************************/

