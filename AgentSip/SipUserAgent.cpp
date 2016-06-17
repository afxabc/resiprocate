
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "SipUserAgent.hxx"
#include "MySipDialer.hxx"

using namespace std;

SipUserAgent::SipUserAgent():mSipDialer(new MySipDialer(this))
{
}

SipUserAgent::~SipUserAgent()
{
	delete mSipDialer;
}

int SipUserAgent::GetMicSoundLevel()
{
	return mSipDialer->GetMicSoundLevel();
}

int SipUserAgent::GetSpkSoundLevel()
{
	return mSipDialer->GetSpkSoundLevel();
}

int SipUserAgent::GetAudioInDevTotal()
{
	return mSipDialer->GetAudioInDevTotal();
}

int SipUserAgent::GetAudioOutDevTotal()
{
	return mSipDialer->GetAudioOutDevTotal();
}

std::string SipUserAgent::GetAudioOutDevName(int nDeviceId)
{
	return mSipDialer->GetAudioOutDevName(nDeviceId);
}

std::string SipUserAgent::GetAudioInDevName(int nDeviceId)
{
	return mSipDialer->GetAudioInDevName(nDeviceId);
}

int SipUserAgent::GetVideoDevTotal()
{
	return mSipDialer->GetVideoDevTotal();
}

std::string SipUserAgent::GetVideoInDevName(int nDeviceId)
{
	return mSipDialer->GetVideoInDevName(nDeviceId);
}

bool SipUserAgent::SelectAudioOutDev(int nDeviceId)
{
	return mSipDialer->SelectAudioOutDev(nDeviceId);
}

bool SipUserAgent::SelectAudioInDev(int nDeviceId)
{
	return mSipDialer->SelectAudioInDev(nDeviceId);
}

bool SipUserAgent::Initialize(bool bBindToListenIP, 
					std::string sListenIP, 
					unsigned int nListenPort, 
					std::string sFromURI, 
					std::string sSIPOutBoundProxy, 
					std::string sSIPProxy, 
					std::string sLoginId, 
					std::string sLoginPwd, 
					bool bUseSoundDevice, 
					int nTotalLine)
{
	return mSipDialer->Initialize(bBindToListenIP, 
						sListenIP, 
						 nListenPort, 
						sFromURI, 
						sSIPOutBoundProxy, 
						sSIPProxy, 
						sLoginId, 
						sLoginPwd, 
						 bUseSoundDevice, 
						nTotalLine);
}

void SipUserAgent::UnInitialize()
{
	return mSipDialer->UnInitialize();
}

bool SipUserAgent::RegisterToProxy(int nExpire)
{
	return mSipDialer->RegisterToProxy(nExpire);
}

bool SipUserAgent::UnRegisterToProxy()
{
	return mSipDialer->UnRegisterToProxy();
}
	
bool SipUserAgent::OpenLine(int nLineNo, bool bBindToRTPRxIP, std::string sRTPRxIP, unsigned int nRTPRxPort)
{
	return mSipDialer->OpenLine(nLineNo, bBindToRTPRxIP, sRTPRxIP, nRTPRxPort);
}

bool SipUserAgent::CloseLine(int nLineNo)
{
	return mSipDialer->CloseLine(nLineNo);
}

bool SipUserAgent::SetOutBoundProxy(std::string sSIPOutBoundProxy, unsigned int nSIPOutBoundPort)
{
	return mSipDialer->SetOutBoundProxy( sSIPOutBoundProxy, nSIPOutBoundPort );
}

bool SipUserAgent::SetLicenceKey(std::string sLicenceKey)
{
	return mSipDialer->SetLicenceKey( sLicenceKey);
}

long SipUserAgent::GetVaxObjectError()
{
	return mSipDialer->GetVaxObjectError();
}
				
bool SipUserAgent::Connect(int nLineNo, std::string sToURI, int nInputDeviceId, int nOutputDeviceId)
{
	return mSipDialer->Connect(nLineNo,sToURI,nInputDeviceId,nOutputDeviceId);
}

bool SipUserAgent::Disconnect(int nLineNo)
{
	return mSipDialer->Disconnect(nLineNo);
}

bool SipUserAgent::AcceptCall(int nLineNo, std::string sCallId, int nInputDeviceId, int nOutputDeviceId)
{
	return mSipDialer->AcceptCall(nLineNo, sCallId, nInputDeviceId, nOutputDeviceId);
}

bool SipUserAgent::RejectCall(std::string sCallId)
{
	return mSipDialer->RejectCall(sCallId);
}
	
bool SipUserAgent::TransferCall(int nLineNo, std::string sToURI)
{
	return mSipDialer->TransferCall(nLineNo, sToURI);
}

bool SipUserAgent::JoinTwoLine(int nLineNoA, int nLineNoB)
{
	return mSipDialer->JoinTwoLine( nLineNoA, nLineNoB);
}
	
bool SipUserAgent::HoldLine(int nLineNo)
{
	return mSipDialer->HoldLine(nLineNo);
}

bool SipUserAgent::UnHoldLine(int nLineNo)
{
	return mSipDialer->UnHoldLine(nLineNo);
}
	
bool SipUserAgent::IsLineOpen(int nLineNo)
{
	return mSipDialer->IsLineOpen(nLineNo);
}

bool SipUserAgent::IsLineHold(int nLineNo)
{
	return mSipDialer->IsLineHold(nLineNo);
}

bool SipUserAgent::IsLineBusy(int nLineNo)
{
	return mSipDialer->IsLineBusy(nLineNo);
}
	
bool SipUserAgent::EnableKeepAlive(int nSeconds)
{
	return mSipDialer->EnableKeepAlive( nSeconds);
}

void SipUserAgent::DisableKeepAlive()
{
	return mSipDialer->DisableKeepAlive();
}

void SipUserAgent::DeselectAllVoiceCodec()
{
	return mSipDialer->DeselectAllVoiceCodec();
}

void SipUserAgent::SelectAllVoiceCodec()
{
	return mSipDialer->SelectAllVoiceCodec();
}

bool SipUserAgent::SelectVoiceCodec(int nCodecNo)
{
	return mSipDialer->SelectVoiceCodec( nCodecNo);
}

bool SipUserAgent::DeselectVoiceCodec(int nCodecNo)
{
	return mSipDialer->DeselectVoiceCodec( nCodecNo);
}

std::string SipUserAgent::GetMyIP()
{
	return mSipDialer->GetMyIP();
}
	
bool SipUserAgent::DigitDTMF(int nLineNo, std::string sDigit)
{
	return mSipDialer->DigitDTMF(nLineNo, sDigit);
}

bool SipUserAgent::SetDTMFVolume(int nVolume)
{
	return mSipDialer->SetDTMFVolume(nVolume);
}

int SipUserAgent::GetDTMFVolume()
{
	return mSipDialer->GetDTMFVolume();
}

bool SipUserAgent::EnableForceInbandDTMF(int nLineNo)
{
	return mSipDialer->EnableForceInbandDTMF(nLineNo);
}

bool SipUserAgent::DisableForceInbandDTMF(int nLineNo)
{
	return mSipDialer->DisableForceInbandDTMF(nLineNo);
}

bool SipUserAgent::MuteMic(bool bMute)
{
	return mSipDialer->MuteMic(bMute);
}

bool SipUserAgent::MuteSpk(bool bMute)
{
	return mSipDialer->MuteSpk(bMute);
}
	
int SipUserAgent::GetMicVolume()
{
	return mSipDialer->GetMicVolume();
}

bool SipUserAgent::SetMicVolume(int nVolume)
{
	return mSipDialer->SetMicVolume(nVolume);
}

int SipUserAgent::GetSpkVolume()
{
	return mSipDialer->GetSpkVolume();
}

bool SipUserAgent::SetSpkVolume(int nVolume)
{
	return mSipDialer->SetSpkVolume(nVolume);
}

bool SipUserAgent::EnableMicBoost()
{
	return mSipDialer->EnableMicBoost();
}

bool SipUserAgent::DisableMicBoost()
{
	return mSipDialer->DisableMicBoost();
}

bool SipUserAgent::IsMicBoostEnable()
{
	return mSipDialer->IsMicBoostEnable();
}

bool SipUserAgent::EnableAGC(int nLevel)
{
	return mSipDialer->EnableAGC(nLevel);
}

bool SipUserAgent::DisableAGC()
{
	return mSipDialer->DisableAGC();
}
	
bool SipUserAgent::EnableEchoNoiseCancellation()
{
	return mSipDialer->EnableEchoNoiseCancellation();
}

bool SipUserAgent::DisableEchoNoiseCancellation()
{
	return mSipDialer->DisableEchoNoiseCancellation();
}

bool SipUserAgent::EnableVideoCapture()
{
	return mSipDialer->EnableVideoCapture();
}

bool SipUserAgent::DisableVideoCapture()
{
	return mSipDialer->DisableVideoCapture();
}

bool SipUserAgent::EnableVideoPlay()
{
	return mSipDialer->EnableVideoPlay();
}

bool SipUserAgent::DisableVideoPlay()
{
	return mSipDialer->DisableVideoPlay();
}


bool SipUserAgent::SelectVideoCodec(int nCodecNo)
{
	return mSipDialer->SelectVideoCodec( nCodecNo);
}

bool SipUserAgent::DeselectVideoCodec(int nCodecNo)
{
	return mSipDialer->DeselectVideoCodec( nCodecNo);
}

bool SipUserAgent::SetVideoBitrate(int nRate)
{
	return mSipDialer->SetVideoBitrate( nRate);
}

bool SipUserAgent::IsRecording(int nLineNo)
{
	return mSipDialer->IsRecording(nLineNo);
}

bool SipUserAgent::StartRecording(int nLineNo, int nRecordVoice, bool bRecordCompress)
{
	return mSipDialer->StartRecording(nLineNo, nRecordVoice, bRecordCompress);
}

bool SipUserAgent::StopRecording(int nLineNo)
{
	return mSipDialer->StopRecording(nLineNo);
}

bool SipUserAgent::ResetRecording(int nLineNo)
{
	return mSipDialer->ResetRecording(nLineNo);
}

bool SipUserAgent::SaveRecordingToWaveFile(int nLineNo, std::string sFileName)
{
	return mSipDialer->SaveRecordingToWaveFile(nLineNo, sFileName);
}

bool SipUserAgent::IsWaveFilePlaying(int nLineNo)
{
	return mSipDialer->IsWaveFilePlaying(nLineNo);
}

bool SipUserAgent::PlayWaveOpen(int nLineNo, std::string sFileName)
{
	return mSipDialer->PlayWaveOpen(nLineNo, sFileName);
}

bool SipUserAgent::PlayWaveSkipTo(int nLineNo, int nSeconds)
{
	return mSipDialer->PlayWaveSkipTo(nLineNo, nSeconds);
}

int SipUserAgent::PlayWaveTotalTime(int nLineNo)
{
	return mSipDialer->PlayWaveTotalTime(nLineNo);
}

bool SipUserAgent::PlayWavePause(int nLineNo)
{
	return mSipDialer->PlayWavePause(nLineNo);
}

bool SipUserAgent::PlayWaveStart(int nLineNo, bool bListen)
{
	return mSipDialer->PlayWaveStart(nLineNo, bListen);
}

bool SipUserAgent::PlayWaveStop(int nLineNo)
{
	return mSipDialer->PlayWaveStop(nLineNo);
}

int SipUserAgent::PlayWavePosition(int nLineNo)
{
	return mSipDialer->PlayWavePosition(nLineNo);
}

void SipUserAgent::EnableDonotDisturb()
{
	return mSipDialer->EnableDonotDisturb();
}

void SipUserAgent::DisableDonotDisturb()
{
	return mSipDialer->DisableDonotDisturb();
}

bool SipUserAgent::SetTOS(int nLineNo, int nValue)
{
	return mSipDialer->SetTOS(nLineNo, nValue);
}

int SipUserAgent::GetTOS(int nLineNo)
{
	return mSipDialer->GetTOS(nLineNo);
}

int SipUserAgent::GetOutboundCodec(int nLineNo)
{
	return mSipDialer->GetOutboundCodec(nLineNo);
}

int SipUserAgent::GetInboundCodec(int nLineNo)
{
	return mSipDialer->GetInboundCodec(nLineNo);
}

std::string SipUserAgent::GetVersionFile()
{
	return mSipDialer->GetVersionFile();
}

std::string SipUserAgent::GetVersionSDK()
{
	return mSipDialer->GetVersionSDK();
}

void SipUserAgent::SetLogType(const string type, const char *logs)
{
	return mSipDialer->SetLogType(type, logs);
}

