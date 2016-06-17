
#ifndef MYSIPDIALER_H
#define MYSIPDIALER_H

#include <string>

#include "resip/stack/NameAddr.hxx"
#include "resip/stack/Uri.hxx"
#include "rutil/Data.hxx"
#include "resip/dum/ClientAuthManager.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/stack/SipStack.hxx"
#include "rutil/SharedPtr.hxx"
#include "resip/stack/StackThread.hxx"
#include "resip/dum/DumThread.hxx"
#include "AppThread.hxx"
#include "SipUserAgent.hxx"
#include "DialerConfiguration.hxx"

#define MAX_USERS 8

class DialInstance;
class RegisterInstance;


class MySipDialer
{
public:
	MySipDialer(SipUserAgent *agent);
	virtual ~MySipDialer();

	std::string getFullFilename();

private:
	bool mySipRun(bool bBindToListenIP, 
					std::string sListenIP, 
					unsigned int nListenPort);

public:
	int GetMicSoundLevel();
	int GetSpkSoundLevel();
	int GetAudioInDevTotal();
	int GetAudioOutDevTotal();
	std::string GetAudioOutDevName(int nDeviceId);
	std::string GetAudioInDevName(int nDeviceId);

	int GetVideoDevTotal();
	std::string GetVideoInDevName(int nDeviceId);
	
	bool SelectAudioOutDev(int nDeviceId);/*add*/
	bool SelectAudioInDev(int nDeviceId);/*add*/

	bool MuteMic(bool bMute);
	bool MuteSpk(bool bMute);
	int GetMicVolume();
	bool SetMicVolume(int nVolume);
	int GetSpkVolume();
	bool SetSpkVolume(int nVolume);

	/*initial sip stack*/
	bool Initialize(bool bBindToListenIP, 
					std::string sListenIP, 
					unsigned int nListenPort, 
					std::string sFromURI, 
					std::string sSIPOutBoundProxy, 
					std::string sSIPProxy, 
					std::string sLoginId, 
					std::string sLoginPwd, 
					bool bUseSoundDevice, 
					int nTotalLine);
	void UnInitialize();

	bool RegisterToProxy(int nExpire);
	bool UnRegisterToProxy();
	
	bool OpenLine(int nLineNo, bool bBindToRTPRxIP, std::string sRTPRxIP, unsigned int nRTPRxPort);
	bool CloseLine(int nLineNo);

	bool SetOutBoundProxy(std::string sSIPOutBoundProxy, unsigned int nSIPOutBoundPort);/*add*/
	bool SetLicenceKey(std::string sLicenceKey);
	long GetVaxObjectError();

	bool Connect(int nLineNo, std::string sToURI, int nInputDeviceId, int nOutputDeviceId);
	bool Disconnect(int nLineNo);
	bool AcceptCall(int nLineNo, std::string sCallId, int nInputDeviceId, int nOutputDeviceId);
	bool RejectCall(std::string sCallId);
	
	bool TransferCall(int nLineNo, std::string sToURI);
	bool JoinTwoLine(int nLineNoA, int nLineNoB);
	
	bool HoldLine(int nLineNo);
	bool UnHoldLine(int nLineNo);
	
	bool IsLineOpen(int nLineNo);
	bool IsLineHold(int nLineNo);
	bool IsLineBusy(int nLineNo);
	
	bool EnableKeepAlive(int nSeconds);
	void DisableKeepAlive();

	void DeselectAllVoiceCodec();
	void SelectAllVoiceCodec();
	bool SelectVoiceCodec(int nCodecNo);
	bool DeselectVoiceCodec(int nCodecNo);

	std::string GetMyIP();
	
	bool DigitDTMF(int nLineNo, std::string sDigit);
	bool SetDTMFVolume(int nVolume);
	int GetDTMFVolume();

	bool EnableForceInbandDTMF(int nLineNo);
	bool DisableForceInbandDTMF(int nLineNo);

	bool EnableMicBoost();
	bool DisableMicBoost();
	bool IsMicBoostEnable();

	bool EnableAGC(int nLevel);
	bool DisableAGC();

	/*for video*/
	bool EnableVideoCapture();
	bool DisableVideoCapture();
	bool EnableVideoPlay();
	bool DisableVideoPlay();
	bool SelectVideoCodec(int nCodecNo);
	bool DeselectVideoCodec(int nCodecNo);
	bool SetVideoBitrate(int nRate);/*1=128k;2=256k;3=512k;others=2-256k*/

	bool EnableEchoNoiseCancellation();
	bool DisableEchoNoiseCancellation();

	bool IsRecording(int nLineNo);
	bool StartRecording(int nLineNo, int nRecordVoice, bool bRecordCompress);
	bool StopRecording(int nLineNo);
	bool ResetRecording(int nLineNo);
	bool SaveRecordingToWaveFile(int nLineNo, std::string sFileName);

	bool IsWaveFilePlaying(int nLineNo);
	bool PlayWaveOpen(int nLineNo, std::string sFileName);
	bool PlayWaveSkipTo(int nLineNo, int nSeconds);
	int PlayWaveTotalTime(int nLineNo);
	bool PlayWavePause(int nLineNo);
	bool PlayWaveStart(int nLineNo, bool bListen);
	bool PlayWaveStop(int nLineNo);
	int PlayWavePosition(int nLineNo);

	void EnableDonotDisturb();
	void DisableDonotDisturb();

	bool SetTOS(int nLineNo, int nValue);
	int GetTOS(int nLineNo);

	int GetOutboundCodec(int nLineNo);
	int GetInboundCodec(int nLineNo);

	std::string GetVersionFile();
	std::string GetVersionSDK();

	virtual void OnTryingToRegister() ;
	virtual void OnFailToRegister() ;
	virtual void OnSuccessToRegister() ;

	virtual void OnTryingToReRegister() ;
	virtual void OnFailToReRegister() ;
	virtual void OnSuccessToReRegister() ;
	
	virtual void OnTryingToUnRegister() ;
	virtual void OnFailToUnRegister() ;
	virtual void OnSuccessToUnRegister() ;
		
	virtual void OnConnecting(int nLineNo) ;
	virtual void OnSuccessToConnect(int nLineNo, std::string sToRTPIP, unsigned int nToRTPPort) ;
	virtual void OnFailToConnect(int nLineNo) ;
		
	virtual void OnDisconnectCall(int nLineNo) ;
	virtual void OnCallTransferAccepted(int nLineNo) ;
	virtual void OnPlayWaveDone(int nLineNo) ;
	virtual void OnDTMFDigit(int nLineNo, std::string sDigit) ;
	
	virtual void OnMsgNOTIFY(std::string sMsg) ;
	virtual void OnVoiceMailMsg(bool bIsMsgWaiting, 
		unsigned long dwNewMsgCount, 
		unsigned long dwOldMsgCount, 
		unsigned long dwNewUrgentMsgCount, 
		unsigned long dwOldUrgentMsgCount, 
		std::string sMsgAccount) ;
	
	virtual void OnIncomingCall(std::string sCallId, 
		std::string sDisplayName, 
		std::string sUserName, 
		std::string sFromURI, 
		std::string sToURI) ;

	virtual void OnIncomingCallRingingStart(std::string sCallId);
	virtual void OnIncomingCallRingingStop(std::string sCallId);
	virtual void OnIncomingCallMissed(std::string sCallId);
	virtual void OnHoldCallMessage(int nLineNo);
	virtual void OnCallTransferStart(int nLineNo, std::string sUri);
	virtual void OnCallTransferSuccess(int nLineNo);
	virtual void OnCallTransferFail(int nLineNo);
		
	virtual void OnProvisionalResponse(int nLineNo, int nStatusCode, std::string sReasonPhrase) ;
	virtual void OnRedirectionResponse(int nLineNo, int nStatusCode, std::string sReasonPhrase, std::string sContact);
	virtual void OnRequestFailureResponse(int nLineNo, int nStatusCode, std::string sReasonPhrase);
	virtual void OnServerFailureResponse(int nLineNo, int nStatusCode, std::string sReasonPhrase);
	virtual void OnGeneralFailureResponse(int nLineNo, int nStatusCode, std::string sReasonPhrase) ;

	/*for video*/
	virtual void OnVideoStartMessage() ;
	virtual void OnVideoStopMessage() ;
	/*fmt:  MS_YUV420P=0,MS_YUYV,MS_RGB24,MS_MJPEG,MS_UYVY,MS_YUY2,*/
	virtual void OnVideoPaintMessage(int fmt, int wide, int high, unsigned char* buf, int le) ;

	virtual void OnIncomingDiagnostic(std::string sMsgSIP, std::string sFromIP, unsigned int nFromPort);
	virtual void OnOutgoingDiagnostic(std::string sMsgSIP, std::string sToIP, unsigned int nToPort) ;

	//reserved
	std::string getDisplayname() ;
	std::string getName() ;
	std::string getProxy() ;
	std::string getPassword() ;

	void SetLogType(const std::string type, const char *logs);
	
	SipUserAgent *mAgent;

private:
	resip::SipStack *mStack;
	resip::DialogUsageManager *mDum;
#if 0
	resip::StackThread *stackThread;
	resip::DumThread *dumThread;
#endif

	DialerConfiguration *dc;
	DialInstance *di;
	RegisterInstance *ri;

	DumRecurringTask* dumRecur;
	AppThread *appThread;

	bool initial;
};

#endif
