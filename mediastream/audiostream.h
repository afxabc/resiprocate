/*********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: audiostream
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\audiostream.h
*********************************************************************/


#ifndef AudioStream_H 

#define AudioStream_H 

#include <string> 
#include "fileplayer.h"
#include "msticker.h"
#include "jrtplib/rtpsession.h"
// dependency msticker 


using namespace resip;

class AppSession;
//----------------------------------------------------------------------------
// audiostream.h                                                                  
//----------------------------------------------------------------------------

//## package Default 
#define MAX_RTP_SIZE	1500

class WincardRead;
class WincardWrite;
class AudioEncode;
class RtpWrite;
class RtpRead;
class AudioDecode;
class FilePlayer;
class AudioRecord;
class AudioPlayer;
class SpeexEc;
class Dtmf;
class Agc;
class ProcessorChain;
class AudioConf;

//## class audiostream 
class AudioStream  
{

////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    AudioStream();
    
    //## auto_generated 
    virtual ~AudioStream();

	RTPSession *create_rtpsession(int locport);
	int audiostream_start(const char *remip,
								   int remport,
								   unsigned char payload,
								   int capId,
								   int playId,
								   bool ec,
								   int gc,
								   bool boost);
	int receive_mediastream(unsigned char payload,int playId);
	void audiostream_stop();
	bool audiostream_alive(int timeout);

	void winsndcardDect(std::string& outname, std::string& inname);
	void ringstream_start(const char* file,int playId);
	void ringstream_stop();

	bool IsRecording();
    bool StartRecording(int nRecordVoice, bool bRecordCompress);
    bool StopRecording();
    bool ResetRecording();
    bool SaveRecordingToWaveFile(Data sFileName);

    bool IsWaveFilePlaying();
    bool PlayWaveOpen(Data sFileName);
    bool PlayWaveSkipTo(int nSeconds);
    int  PlayWaveTotalTime();
    bool PlayWavePause();
    bool PlayWaveStart(bool bListen);
    bool PlayWaveStop();
    int  PlayWavePosition();

	int GetMicSoundLevel();
	int GetSpkSoundLevel();
	int GetMicVolume();
	bool SetMicVolume(int nVolume);
	int GetSpkVolume();
	bool SetSpkVolume(int nVolume);
	void SetMicMute(bool bMute);
	void SetSpkMute(bool bMute);

	void SetEchoCancel(bool ec);
	void SetAGC(int gc);
	void SetMicBoost(bool boost);
	void SendDTMF(Data digit);

	void SetConference(AudioConf *confer) {this->conf = confer; };
	bool isRunning() {return runflag;};
	bool isTalking() {return talking;};

	virtual MSTicker *getTicker() = 0;

public:
//	MSTicker *ticker;
	RTPSession *session;
	WincardRead* cardread;
	AudioEncode* encode;
	RtpWrite* rtpwrite;
	RtpRead* rtpread;
	AudioDecode* decode;
	WincardWrite* cardwrite;
	FilePlayer* fileread;
	AudioPlayer* fileplayer;
	AudioRecord* filerecorder;
	SpeexEc* speexec;
	Dtmf* dtmf;
	Agc* agc;

	ProcessorChain* audioprocess;
	ProcessorChain* audioring;
	ProcessorChain* localring;

	AudioConf *conf;

	bool runflag;
	bool talking;
//	msfilter *dtmf;
//	msfilter *ec;/*echo canceler*/
	unsigned int last_packet_count;
	time_t last_packet_time;

	int spklevel;   
	bool spkmute;

	int miclevel;  
	bool micmute;
};


#endif  
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\audiostream.h
*********************************************************************/

