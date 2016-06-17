
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "MySipDialer.hxx"
#include "resip/stack/NameAddr.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/dum/ClientInviteSession.hxx"
#include "DialInstance.hxx"
#include "RegisterInstance.hxx"
#include "MyAppDialogSet.hxx"
#include "DumRecurringTask.hxx"

#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace resip;
using namespace std;

MySipDialer::MySipDialer(SipUserAgent *agent):
mAgent(agent),
dc(NULL),
mStack(NULL),
mDum(NULL),
di(NULL),
ri(NULL),
#if 0
stackThread(NULL),
dumThread(NULL),
#endif
dumRecur(NULL),
appThread(NULL),
initial(false)
{
}

MySipDialer::~MySipDialer()
{
#if 0
	ofstream out(getFullFilename().c_str());
	if(!out.is_open())
      assert(0); // FIXME

   dc->saveStream(out);
   out.close();

   if(stackThread!=NULL)
   {
	   stackThread->shutdown();
	   stackThread->join();

	   delete stackThread;
   }
   if(dumThread!=NULL)
   {
	   dumThread->shutdown();
	   dumThread->join();

	   delete dumThread;
   }
#endif
   if(initial)
   {
	   UnInitialize();
   }
  
}

string MySipDialer::getFullFilename()
{
#ifdef WIN32
//	char home_direct[255];
//	GetCurrentDirectory(255,(LPWSTR)home_direct);
   char *home_drive = getenv("HOMEDRIVE");
   assert(home_drive); // FIXME
   char *home_path = getenv("HOMEPATH");
   assert(home_path); // FIXME
   char buf[MAX_PATH];
   int len=GetCurrentDirectoryA(MAX_PATH,buf);
   
   string full_filename;
   if(len>0)
   {
	   buf[len]=0;
	   full_filename=buf;
	   full_filename+="/sipdial.cfg";
   }
   else
   {
	   full_filename="N:/resiprocate-1.4/apps/sipdial/sipdial.cfg";
   }
   return full_filename;
#else   
   char *home_dir = getenv("HOME");
   assert(home_dir); // FIXME
   string full_filename(string(home_dir) + string("/.sipdial/sipdial.cfg"));
   return full_filename;
#endif
}
#if 0
void MySipDialer::mySipStart() 
{   
}
#endif

bool MySipDialer::mySipRun(bool bBindToListenIP, 
					std::string sListenIP, 
					unsigned int nListenPort) 
{
	if(initial==true) return false;

	mDum->addTransport(UDP, bBindToListenIP ? nListenPort : 5060, V4);
   SharedPtr<MasterProfile> masterProfile = SharedPtr<MasterProfile>(new MasterProfile);
   
   masterProfile->addSupportedMimeType(resip::NOTIFY, resip::Mime("message", "sipfrag"));
   masterProfile->addSupportedMethod(resip::NOTIFY);
   masterProfile->addSupportedMethod(resip::REFER);
   masterProfile->addSupportedMethod(resip::SUBSCRIBE);
   masterProfile->addSupportedMethod(resip::RESPONSE);
   masterProfile->addSupportedMethod(resip::MESSAGE);
   masterProfile->addSupportedMethod(resip::PUBLISH);
   mDum->setMasterProfile(masterProfile);

   auto_ptr<AppDialogSetFactory> myAppDialogSetFactory(new MyAppDialogSetFactory);
   mDum->setAppDialogSetFactory(myAppDialogSetFactory);

   auto_ptr<ClientAuthManager> clientAuth(new ClientAuthManager);
   mDum->setClientAuthManager(clientAuth);

   dc->loadConfigure();

   mDum->getMasterProfile()->setDefaultFrom(NameAddr(dc->getDialerIdentity()));
/*
   stackThread=new StackThread(*mStack);
   dumThread = new DumThread(*mDum);
   
   // Make it all go 
   stackThread->run();
   dumThread->run();*/
   dumRecur=new DumRecurringTask(*mStack,*mDum);

   di=new DialInstance(*dc);
   di->setMySipDialer(this);
   di->setSipStack(mStack);
   di->setDum(mDum);
   di->execute();

   appThread=new AppThread(di, dumRecur);
   appThread->run();

   if(dc->getSendRegister()>=0)
   {
	   ri=new RegisterInstance(*dc);
	   ri->mSipDialer=this;
	   ri->setSipStack(mStack);
	   ri->setDum(mDum);

	   ri->execute();
   }
   OnMsgNOTIFY("Ready ");
   initial=true;
   return true;
}

int MySipDialer::GetMicSoundLevel()
{
	if( !initial ) return 0;

	return di->GetMicSoundLevel();
}

int MySipDialer::GetSpkSoundLevel()
{
	if( !initial ) return 0;

	return di->GetSpkSoundLevel();
}

int MySipDialer::GetAudioInDevTotal()
{
	if( !initial ) return 0;

	return di->getMSSndCard()->GetAudioCapDevTotal();
}

int MySipDialer::GetAudioOutDevTotal()
{
	if( !initial ) return 0;

	return di->getMSSndCard()->GetAudioPlayDevTotal();
}

std::string MySipDialer::GetAudioOutDevName(int nDeviceId)
{
	if( !initial ) return string("");

	Data str=di->getMSSndCard()->GetPlayCardName(nDeviceId);
	return str.data();
}

std::string MySipDialer::GetAudioInDevName(int nDeviceId)
{
	if( !initial ) return string("");

	Data str=di->getMSSndCard()->GetCapCardName(nDeviceId);
	return str.data();
}

int MySipDialer::GetVideoDevTotal()
{
	if( !initial ) return 0;

	return di->getMSSndCard()->GetVideoDevTotal();
}

std::string MySipDialer::GetVideoInDevName(int nDeviceId)
{
	if( !initial ) return string("");

	Data str=di->getMSSndCard()->GetVideoCapName(nDeviceId);
	return str.data();
}

bool MySipDialer::SelectAudioOutDev(int nDeviceId)
{
	if( !initial ) return false;

	dc->setOutAudioDev(nDeviceId);
	di->setOutAudioDevice(nDeviceId);
	return true;
}

bool MySipDialer::SelectAudioInDev(int nDeviceId)
{
	if( !initial ) return false;

	dc->setInAudioDev(nDeviceId);
	return true;
}

bool MySipDialer::Initialize(bool bBindToListenIP, 
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
	if( ( sSIPProxy.length() == 0 )||(sLoginId.length() == 0) )
	{
		OnMsgNOTIFY("Not Ready");
		return false;
	}
	dc = new DialerConfiguration();
	mStack = new SipStack();
	mDum = new DialogUsageManager(*mStack);

	dc->setAuthRealm(resip::Data(sSIPProxy));
	dc->setAuthUser(resip::Data(sLoginId));
	dc->setAuthPassword(resip::Data(sLoginPwd));
	dc->setDisplayName(resip::Data(sFromURI));
	dc->setOutboundProxy(resip::Data(sSIPOutBoundProxy));

	if(nTotalLine > MAX_USERS)
	{
		OnMsgNOTIFY("the number of total line is excessive ");
		assert(0);
		return false;
	}
	bool flag=mySipRun( bBindToListenIP, 
					 sListenIP, 
					 nListenPort);

	/*get system information*/
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	MEMORYSTATUS MemoryStatus;
	MemoryStatus.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&MemoryStatus);
	OSVERSIONINFO versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	GetVersionEx (&versionInfo);
	SYSTEMTIME systime;
	GetSystemTime(&systime);
	if(0/*(systime.wYear>2010)||((systime.wYear==2010)&&(systime.wMonth>6))*/)
	{
		OnMsgNOTIFY("this version is out of date ");
		assert(0);
		return false;
	}

	InfoLog(<<"SYSINFO->PROCESSOR_ARCH    "<<sysinfo.wProcessorArchitecture); 
	InfoLog(<<"SYSINFO->PROCESSOR_LEVEL    "<<sysinfo.wProcessorLevel);
	InfoLog(<<"SYSINFO->PROCESSOR_REVISION    "<<sysinfo.wProcessorRevision);
	InfoLog(<<"SYSINFO->PROCESSOR_TYPE    "<<sysinfo.dwProcessorType);
	InfoLog(<<"SYSINFO->PAGE_SIZE    "<<sysinfo.dwPageSize);
	InfoLog(<<"SYSINFO->NUM_OF_PROCESSOR    "<<sysinfo.dwNumberOfProcessors);
	InfoLog(<<"SYSINFO->ALLOC_GRANULARITY    "<<sysinfo.dwAllocationGranularity);

	InfoLog(<<"MEMORYSTATUS->TOTAL_PHY    "<<MemoryStatus.dwTotalPhys);
	InfoLog(<<"MEMORYSTATUS->AVAIL_PHY    "<<MemoryStatus.dwAvailPhys);
	InfoLog(<<"MEMORYSTATUS->TOTAL_VRT    "<<MemoryStatus.dwTotalVirtual);
	InfoLog(<<"MEMORYSTATUS->AVAIL_VRT    "<<MemoryStatus.dwAvailVirtual);
	InfoLog(<<"MEMORYSTATUS->TOTAL_PAG    "<<MemoryStatus.dwTotalPageFile);
	InfoLog(<<"MEMORYSTATUS->AVAIL_PAG    "<<MemoryStatus.dwAvailPageFile);
	InfoLog(<<"MEMORYSTATUS->LOAD    "<<MemoryStatus.dwMemoryLoad);

	InfoLog(<<"OSVERSIONINFO->PLATFORM    "<<versionInfo.dwPlatformId);
	InfoLog(<<"OSVERSIONINFO->MAJOR_VERSION    "<<versionInfo.dwMajorVersion);
	InfoLog(<<"OSVERSIONINFO->MINOR_VERSION    "<<versionInfo.dwMinorVersion);
	InfoLog(<<"OSVERSIONINFO->BUILD_NUM    "<<versionInfo.dwBuildNumber);
	
/*	char tmp[30];
	for(int i=0;i<sizeof(tmp);i++)
	{
		tmp[i] = versionInfo.szCSDVersion[i];
	}*/
	InfoLog(<<"OSVERSIONINFO->SERVICE_PACK    "<<versionInfo.szCSDVersion);
	
	return flag;
}

void MySipDialer::UnInitialize()
{
	if( initial == false)
	{
		return;
	}
	if(appThread!=NULL)
	{
		appThread->shutdown();
		appThread->join();

		delete appThread;
	}
   if(dumRecur!=NULL)
   {
	   delete dumRecur;
   }

   if(di)
   {
	   delete di;
	   di=NULL;
   }
   if(ri)
   {
	   delete ri;
	   ri=NULL;
   }
    
   delete mDum;
   delete mStack;
   delete dc;

   OnMsgNOTIFY("Not Ready ");
   initial=false;
}

bool MySipDialer::RegisterToProxy(int nExpire)
{
	if( !initial ) return false;

	dc->setSendRegister(nExpire);
	if(ri!=NULL)
	{
		ri->sendRegister(nExpire);
	}
	else
	{
		ri=new RegisterInstance(*dc);
		ri->mSipDialer=this;
		ri->setSipStack(mStack);
		ri->setDum(mDum);

		ri->execute();
	}
	return true;
}

bool MySipDialer::UnRegisterToProxy()
{
	if( !initial ) return false;

	dc->setSendRegister(-1);
	if(ri!=NULL)
	{
		ri->sendRegister(-1);
	}
	return true;
}
	
bool MySipDialer::OpenLine(int nLineNo, bool bBindToRTPRxIP, std::string sRTPRxIP, unsigned int nRTPRxPort)
{
	if( !initial ) return false;

	return di->OpenLine(nLineNo, bBindToRTPRxIP, Data(sRTPRxIP), nRTPRxPort);
}

bool MySipDialer::CloseLine(int nLineNo)
{
	if( !initial ) return false;

	return di->CloseLine(nLineNo);
}

bool MySipDialer::SetOutBoundProxy(std::string sSIPOutBoundProxy, unsigned int nSIPOutBoundPort)
{
	if( !initial ) return false;

	dc->setOutboundProxy( Data(sSIPOutBoundProxy) );
	dc->setOutboundPort( Data(nSIPOutBoundPort) );
	return true;
}

bool MySipDialer::SetLicenceKey(std::string sLicenceKey)
{
	if( !initial ) return false;

	return true;
}

long MySipDialer::GetVaxObjectError()
{
	if( !initial ) return 0L;

	return 0L;
}
				
bool MySipDialer::Connect(int nLineNo, std::string sToURI, int nInputDeviceId, int nOutputDeviceId)
{
	if( !initial ) return false;

	return di->sendInvite(nLineNo,Data(sToURI),nInputDeviceId,nOutputDeviceId);
}

bool MySipDialer::Disconnect(int nLineNo)
{
	if( !initial ) return false;

	di->sendHangup(nLineNo);
	return true;
}

bool MySipDialer::AcceptCall(int nLineNo, std::string sCallId, int nInputDeviceId, int nOutputDeviceId)
{
	if( !initial ) return false;

	di->sendAnswer(nLineNo, Data(sCallId), nInputDeviceId, nOutputDeviceId);
	return true;
}

bool MySipDialer::RejectCall(std::string sCallId)
{
	if( !initial ) return false;

	di->sendReject(Data(sCallId));
	return true;
}
	
bool MySipDialer::TransferCall(int nLineNo, std::string sToURI)
{
	if( !initial ) return false;

	return di->Transfer(nLineNo, Data(sToURI));
}

bool MySipDialer::JoinTwoLine(int nLineNoA, int nLineNoB)
{
	if( !initial ) return false;

	return true;
}
	
bool MySipDialer::HoldLine(int nLineNo)
{
	if( !initial ) return false;

	return di->HoldLine(nLineNo);
}

bool MySipDialer::UnHoldLine(int nLineNo)
{
	if( !initial ) return false;

	return di->UnHoldLine(nLineNo);
}
	
bool MySipDialer::IsLineOpen(int nLineNo)
{
	if( !initial ) return false;

	return di->LineIsOpen(nLineNo);
}

bool MySipDialer::IsLineHold(int nLineNo)
{
	if( !initial ) return false;

	return di->LineIsHold(nLineNo);
}

bool MySipDialer::IsLineBusy(int nLineNo)
{
	if( !initial ) return false;

	return di->LineIsBusy(nLineNo);
}
	
bool MySipDialer::EnableKeepAlive(int nSeconds)
{
	if( !initial ) return false;

	return true;
}

void MySipDialer::DisableKeepAlive()
{
	if( !initial ) return;

}

void MySipDialer::DeselectAllVoiceCodec()
{
	if( !initial ) return;

	dc->setCodecPCMU(false);
	dc->setCodecPCMA(false);
	dc->setCodecGSM(false);
	dc->setCodeciLBC(false);
	dc->setCodecG723(false);
	dc->setCodecG729(false);
}

void MySipDialer::SelectAllVoiceCodec()
{
	if( !initial ) return;

	dc->setCodecPCMU(true);
	dc->setCodecPCMA(true);
	dc->setCodecGSM(true);
	dc->setCodeciLBC(true);
	dc->setCodecG723(true);
	dc->setCodecG729(true);
}

bool MySipDialer::SelectVoiceCodec(int nCodecNo)
{
	if( !initial ) return false;

	switch(nCodecNo)
	{
	case PCMU:
		dc->setCodecPCMU(true);
		return true;
	case PCMA:
		dc->setCodecPCMA(true);
		return true;
	case G723:
		dc->setCodecG723(true);
		return true;
	case G729:
		dc->setCodecG729(true);
		return true;
	case GSM:
		dc->setCodecGSM(true);
		return true;
	case iLBC:
		dc->setCodeciLBC(true);
		return true;
	default:
		return false;
	}
	return true;
}

bool MySipDialer::DeselectVoiceCodec(int nCodecNo)
{
	if( !initial ) return false;

	switch(nCodecNo)
	{
	case PCMU:
		dc->setCodecPCMU(false);
		return true;
	case PCMA:
		dc->setCodecPCMA(false);
		return true;
	case GSM:
		dc->setCodecGSM(false);
		return true;
	case iLBC:
		dc->setCodeciLBC(false);
		return true;
	case G723:
		dc->setCodecG723(false);
		return true;
	case G729:
		dc->setCodecG729(false);
		return true;
	default:
		return false;
	}
}

std::string MySipDialer::GetMyIP()
{
	if( !initial ) return string("");

	Data addr=dc->getHostAddress();
	return addr.data();
}
	
bool MySipDialer::DigitDTMF(int nLineNo, std::string sDigit)
{
	if( !initial ) return false;

	return di->DigitDTMF(nLineNo, Data(sDigit));
}

bool MySipDialer::SetDTMFVolume(int nVolume)
{
	if( !initial ) return false;

	return di->SetDTMFVolume(nVolume);
}

int MySipDialer::GetDTMFVolume()
{
	if( !initial ) return false;

	return di->GetDTMFVolume();
}

bool MySipDialer::EnableForceInbandDTMF(int nLineNo)
{
	if( !initial ) return false;

	return di->EnableInbandDTMF(nLineNo,true);
}

bool MySipDialer::DisableForceInbandDTMF(int nLineNo)
{
	if( !initial ) return false;

	return di->EnableInbandDTMF(nLineNo,false);
}

bool MySipDialer::MuteMic(bool bMute)
{
	if( !initial ) return false;

	return di->MuteMic(bMute);
}

bool MySipDialer::MuteSpk(bool bMute)
{
	if( !initial ) return false;

	return di->MuteSpk(bMute);
}
	
int MySipDialer::GetMicVolume()
{
	if( !initial ) return false;

	return di->GetMicVolume();
}

bool MySipDialer::SetMicVolume(int nVolume)
{
	if( !initial ) return false;

	return di->SetMicVolume(nVolume);
}

int MySipDialer::GetSpkVolume()
{
	if( !initial ) return false;

	return di->GetSpkVolume();
}

bool MySipDialer::SetSpkVolume(int nVolume)
{
	if( !initial ) return false;

	return di->SetSpkVolume(nVolume);
}

bool MySipDialer::EnableMicBoost()
{
	if( !initial ) return false;

	dc->setMicBoost(true);
	di->setMicBoost(true);
	return true;
}

bool MySipDialer::DisableMicBoost()
{
	if( !initial ) return false;

	dc->setMicBoost(false);
	di->setMicBoost(false);
	return true;
}

bool MySipDialer::IsMicBoostEnable()
{
	if( !initial ) return false;

	return dc->IsMicBoostEnable();
}

bool MySipDialer::EnableAGC(int nLevel)
{
	if( !initial ) return false;

	dc->setAGCLevel(nLevel);
	di->setAGCLevel(nLevel);
	return true;
}

bool MySipDialer::DisableAGC()
{
	if( !initial ) return false;

	dc->setAGCLevel(-1);
	di->setAGCLevel(-1);
	return true;
}
	
bool MySipDialer::EnableEchoNoiseCancellation()
{
	if( !initial ) return false;

	dc->setEchoNoiseCancellation(true);
	di->setEchoNoiseCancellation(true);
	return true;
}

bool MySipDialer::DisableEchoNoiseCancellation()
{
	if( !initial ) return false;

	dc->setEchoNoiseCancellation(false);
	di->setEchoNoiseCancellation(false);
	return true;
}

bool MySipDialer::EnableVideoCapture()
{
	if( !initial ) return false;

#ifdef VIDEO_ENABLE
	dc->setVideoCapture(true);
#endif
//	OnMsgNOTIFY("select VideoCapture ");
	return true;
}

bool MySipDialer::DisableVideoCapture()
{
	if( !initial ) return false;

#ifdef VIDEO_ENABLE
	dc->setVideoCapture(false);
#endif
//	OnMsgNOTIFY("not select VideoCapture ");
	return true;
}

bool MySipDialer::EnableVideoPlay()
{
	if( !initial ) return false;

#ifdef VIDEO_ENABLE
	dc->setVideoPlay(true);
#endif
//	OnMsgNOTIFY("select VideoPlay ");
	return true;
}

bool MySipDialer::DisableVideoPlay()
{
	if( !initial ) return false;

#ifdef VIDEO_ENABLE
	dc->setVideoPlay(false);
#endif
//	OnMsgNOTIFY("not select VideoPlay ");
	return true;
}


bool MySipDialer::SelectVideoCodec(int nCodecNo)
{
	if( !initial ) return false;

#ifdef VIDEO_ENABLE
	switch(nCodecNo)
	{
	case H263_1998:
		dc->setCodecH263_1998(true);
		break;

	default:
		break;
	}
#endif
	return true;
}

bool MySipDialer::DeselectVideoCodec(int nCodecNo)
{
	if( !initial ) return false;

#ifdef VIDEO_ENABLE
	switch(nCodecNo)
	{
	case H263_1998:
		dc->setCodecH263_1998(false);
		break;

	default:
		break;
	}
#endif
	return true;
}

bool MySipDialer::SetVideoBitrate(int nRate)
{
	if( !initial ) return false;

#ifdef VIDEO_ENABLE

#endif
	return true;
}

bool MySipDialer::IsRecording(int nLineNo)
{
	if( !initial ) return false;

	return di->IsRecording(nLineNo);
}

bool MySipDialer::StartRecording(int nLineNo, int nRecordVoice, bool bRecordCompress)
{
	if( !initial ) return false;

	return di->StartRecording(nLineNo, nRecordVoice, bRecordCompress);
}

bool MySipDialer::StopRecording(int nLineNo)
{
	if( !initial ) return false;

	return di->StopRecording(nLineNo);
}

bool MySipDialer::ResetRecording(int nLineNo)
{
	if( !initial ) return false;

	return di->ResetRecording(nLineNo);
}

bool MySipDialer::SaveRecordingToWaveFile(int nLineNo, std::string sFileName)
{
	if( !initial ) return false;

	return di->SaveRecordingToWaveFile(nLineNo, Data(sFileName));
}

bool MySipDialer::IsWaveFilePlaying(int nLineNo)
{
	if( !initial ) return false;

	return di->IsWaveFilePlaying(nLineNo);
}

bool MySipDialer::PlayWaveOpen(int nLineNo, std::string sFileName)
{
	if( !initial ) return false;

	return di->PlayWaveOpen(nLineNo, Data(sFileName));
}

bool MySipDialer::PlayWaveSkipTo(int nLineNo, int nSeconds)
{
	if( !initial ) return false;

	return di->PlayWaveSkipTo(nLineNo, nSeconds);
}

int MySipDialer::PlayWaveTotalTime(int nLineNo)
{
	if( !initial ) return false;

	return di->PlayWaveTotalTime(nLineNo);
}

bool MySipDialer::PlayWavePause(int nLineNo)
{
	if( !initial ) return false;

	return di->PlayWavePause(nLineNo);
}

bool MySipDialer::PlayWaveStart(int nLineNo, bool bListen)
{
	if( !initial ) return false;

	return di->PlayWaveStart(nLineNo, bListen);
}

bool MySipDialer::PlayWaveStop(int nLineNo)
{
	if( !initial ) return false;

	return di->PlayWaveStop(nLineNo);
}

int MySipDialer::PlayWavePosition(int nLineNo)
{
	if( !initial ) return false;

	return di->PlayWavePosition(nLineNo);
}

void MySipDialer::EnableDonotDisturb()
{
	if( !initial ) return;

	dc->setDonotDisturb(true);
}

void MySipDialer::DisableDonotDisturb()
{
	if( !initial ) return;

	dc->setDonotDisturb(false);
}

bool MySipDialer::SetTOS(int nLineNo, int nValue)
{
	if( !initial ) return false;

	return di->SetTOS(nLineNo, nValue);
}

int MySipDialer::GetTOS(int nLineNo)
{
	if( !initial ) return 0;

	return di->GetTOS(nLineNo);
}

int MySipDialer::GetOutboundCodec(int nLineNo)
{
	if( !initial ) return 0;

	return di->GetOutboundCodec(nLineNo);
}

int MySipDialer::GetInboundCodec(int nLineNo)
{
	if( !initial ) return 0;

	return di->GetInboundCodec(nLineNo);
}

std::string MySipDialer::GetVersionFile()
{
	if( !initial ) return string("");

	return NULL;
}

std::string MySipDialer::GetVersionSDK()
{
	if( !initial ) return string("");

	return NULL;
}

void MySipDialer::OnTryingToRegister()
{
	mAgent->OnTryingToRegister();
}

void MySipDialer::OnFailToRegister()
{
	mAgent->OnFailToRegister();
}

void MySipDialer::OnSuccessToRegister() 
{
	mAgent->OnSuccessToRegister();
}


void MySipDialer::OnTryingToReRegister() 
{
	mAgent->OnTryingToReRegister();
}

void MySipDialer::OnFailToReRegister() 
{
	mAgent->OnFailToReRegister();
}

void MySipDialer::OnSuccessToReRegister() 
{
	mAgent->OnSuccessToReRegister();
}
	

void MySipDialer::OnTryingToUnRegister() 
{
	mAgent->OnTryingToUnRegister();
}

void MySipDialer::OnFailToUnRegister() 
{
	mAgent->OnFailToUnRegister();
}

void MySipDialer::OnSuccessToUnRegister() 
{
	mAgent->OnSuccessToUnRegister();
}

void MySipDialer::OnConnecting(int nLineNo) 
{
	mAgent->OnConnecting(nLineNo);
}

void MySipDialer::OnSuccessToConnect(int nLineNo, std::string sToRTPIP, unsigned int nToRTPPort) 
{
	mAgent->OnSuccessToConnect( nLineNo, sToRTPIP, nToRTPPort);
}

void MySipDialer::OnFailToConnect(int nLineNo) 
{
	mAgent->OnFailToConnect(nLineNo);
}
		

void MySipDialer::OnDisconnectCall(int nLineNo) 
{
	mAgent->OnDisconnectCall(nLineNo);
}

void MySipDialer::OnCallTransferAccepted(int nLineNo) 
{
	mAgent->OnCallTransferAccepted(nLineNo);
}

void MySipDialer::OnPlayWaveDone(int nLineNo) 
{
	mAgent->OnPlayWaveDone(nLineNo);
}

void MySipDialer::OnDTMFDigit(int nLineNo, std::string sDigit) 
{
	mAgent->OnDTMFDigit(nLineNo, sDigit);
}
	
	
void MySipDialer::OnMsgNOTIFY(std::string sMsg) 
{
	mAgent->OnMsgNOTIFY(sMsg);
}
	
void MySipDialer::OnVoiceMailMsg(bool bIsMsgWaiting, 
								 unsigned long dwNewMsgCount, 
								 unsigned long dwOldMsgCount, 
								 unsigned long dwNewUrgentMsgCount, 
								 unsigned long dwOldUrgentMsgCount, 
								 std::string sMsgAccount) 
{
	mAgent->OnVoiceMailMsg(bIsMsgWaiting, 
		dwNewMsgCount, 
		dwOldMsgCount, 
		dwNewUrgentMsgCount, 
		dwOldUrgentMsgCount, 
		sMsgAccount);
}
	
	
void MySipDialer::OnIncomingCall(std::string sCallId, std::string sDisplayName, std::string sUserName, std::string sFromURI, std::string sToURI)
{
	mAgent->OnIncomingCall(sCallId, sDisplayName, sUserName, sFromURI, sToURI);
}
	
void MySipDialer::OnIncomingCallRingingStart(std::string sCallId) 
{
	mAgent->OnIncomingCallRingingStart(sCallId);
}
	
void MySipDialer::OnIncomingCallRingingStop(std::string sCallId) 
{
	mAgent->OnIncomingCallRingingStop(sCallId);
}
		
void MySipDialer::OnIncomingCallMissed(std::string sCallId) 
{
	mAgent->OnIncomingCallMissed(sCallId);
}

void MySipDialer::OnHoldCallMessage(int nLineNo)
{
	mAgent->OnHoldCallMessage(nLineNo);
}

void MySipDialer::OnCallTransferStart(int nLineNo, std::string sUri) 
{
	mAgent->OnCallTransferStart(nLineNo, sUri);
}
	
void MySipDialer::OnCallTransferSuccess(int nLineNo)
{
	mAgent->OnCallTransferSuccess(nLineNo);
}

void MySipDialer::OnCallTransferFail(int nLineNo)
{
	mAgent->OnCallTransferFail(nLineNo);
}

void MySipDialer::OnProvisionalResponse(int nLineNo, int nStatusCode, std::string sReasonPhrase) 
{
	mAgent->OnProvisionalResponse(nLineNo, nStatusCode, sReasonPhrase);
}
	
void MySipDialer::OnRedirectionResponse(int nLineNo, int nStatusCode, std::string sReasonPhrase, std::string sContact) 
{
	mAgent->OnRedirectionResponse( nLineNo, nStatusCode, sReasonPhrase, sContact);
}
	
void MySipDialer::OnRequestFailureResponse(int nLineNo, int nStatusCode, std::string sReasonPhrase) 
{
	mAgent->OnRequestFailureResponse( nLineNo, nStatusCode, sReasonPhrase);
}
	
void MySipDialer::OnServerFailureResponse(int nLineNo, int nStatusCode, std::string sReasonPhrase) 
{
	mAgent->OnServerFailureResponse( nLineNo, nStatusCode, sReasonPhrase);
}
	
void MySipDialer::OnGeneralFailureResponse(int nLineNo, int nStatusCode, std::string sReasonPhrase) 
{
	mAgent->OnGeneralFailureResponse( nLineNo, nStatusCode, sReasonPhrase);
}

void MySipDialer::OnVideoStartMessage() 
{
	mAgent->OnVideoStartMessage();
}
	
void MySipDialer::OnVideoStopMessage() 
{
	mAgent->OnVideoStopMessage();
}
	
void MySipDialer::OnVideoPaintMessage(int fmt, int wide, int high, unsigned char* buf, int le) 
{
	mAgent->OnVideoPaintMessage( fmt, wide, high, buf, le);
}

	
void MySipDialer::OnIncomingDiagnostic(std::string sMsgSIP, std::string sFromIP, unsigned int nFromPort) 
{
	mAgent->OnIncomingDiagnostic( sMsgSIP, sFromIP, nFromPort);
}
	
void MySipDialer::OnOutgoingDiagnostic(std::string sMsgSIP, std::string sToIP, unsigned int nToPort) 
{
	mAgent->OnOutgoingDiagnostic( sMsgSIP, sToIP, nToPort);
}


std::string MySipDialer::getDisplayname() 
{
	return std::string(dc->getDialerIdentity().displayName().c_str());
};

std::string MySipDialer::getName() 
{
	return std::string(dc->getDialerIdentity().uri().user().c_str());
};

std::string MySipDialer::getProxy() 
{
	return std::string(dc->getRegIdentity().host().c_str());
};

std::string MySipDialer::getPassword() 
{
	return std::string(dc->getAuthPassword().c_str());
};

void MySipDialer::SetLogType(const string type, const char *logs)
{
	Data logtype(type);
	if(logtype==Data("cout"))
	{
		Log::initialize(logtype.data(), logs, 0);
	}
	else if(logtype==Data("file"))
	{
		Log::initialize(logtype.data(), logs, 0,"resip.log");
	}
	else
	{
		assert(0);
	}
}

