
#ifndef DIALERCONFIGURATION_H
#define DIALERCONFIGURATION_H

#include <iostream>
#include <list>
#include "resip/stack/SdpContents.hxx"
#include "resip/stack/NameAddr.hxx"
#include "resip/stack/Uri.hxx"
#include "rutil/Data.hxx"

#define PACKAGE_SOUND_DIR "N:/resiprocate-1.4/apps"
/* relative path where is stored local ring*/
#define LOCAL_RING "toy.wav"
/* same for remote ring (ringback)*/
#define REMOTE_RING_FR "sweet.wav"
#define REMOTE_RING_US "sweet.wav"

class DialerConfiguration
{
public:
   typedef enum{
	NO_FIREWALL,
	USE_NAT_ADDRESS,
	USE_STUN
   }FirewallPolicy;

   DialerConfiguration();
   virtual ~DialerConfiguration();

   resip::Data getHostAddress();

   void loadStream(std::istream& in);
   void loadConfigure();
   void loadCodecConfig();
   void saveStream(std::ostream& out);

   void setDialerIdentity(const resip::NameAddr& dialerIdentity) { mDialerIdentity = dialerIdentity; };
   void setRegIdentity(const resip::Uri& regIdentity) { mRegIdentity = regIdentity; };

   void setAuthRealm(const resip::Data& authRealm) { mAuthRealm = authRealm; };
   void setAuthUser(const resip::Data& authUser)  { mAuthUser = authUser; };
   void setAuthPassword(const resip::Data& authPassword) { mAuthPassword = authPassword; };
   void setRTPPort(const resip::Data& locPort) { mRtpPort = locPort; };
   
   void setCodecPCMA(const bool status) { mPCMA = status;};
   void setCodecPCMU(const bool status) { mPCMU = status;};
   void setCodecGSM(const bool status) { mGSM = status;};
   void setCodecG723(const bool status) { mG723 = status;};
   void setCodecG729(const bool status) { mG729 = status;};
   void setCodecSpeex(const bool status) { mSpeex = status; };
   void setCodeciLBC(const bool status) { miLBC = status; };
   void setCodecPriority(const int codec) { mCodecPrio = codec; };

   void setLocRealm(const resip::Data& lcoIP)  { mLocRealm = lcoIP; };
   void setListenPort(const resip::Data& lisport)  { mListenPort = lisport; };
   void setDisplayName(const resip::Data& disname)  { mDisplayName = disname; };
   /**/
   void setOutboundProxy(const resip::Data& outproxy)  { mOutboundProxy = outproxy; };
   void setOutboundPort(const resip::Data& outport)  { mOutboundPort = outport; };
   void setOutAudioDev(const int dev)  { mOutAudioDev = dev; };
   void setInAudioDev(const int dev)  { mInAudioDev = dev; };

   void setStunServer(const resip::Data& stunIP)  { mStunServer = stunIP; };
   void setStunPort(const resip::Data& stunPort)  { mStunPort = stunPort; };
   const resip::Uri& getStunIdentity() const { return mStunIdentity; };

   void setPlayFile(const resip::Data& playname)  { mPlayFile = playname; };
   void setRecordFile(const resip::Data& recname)  { mRecordFile = recname; };
   void setSendRegister(const int sendreg)  { mSendRegister = sendreg; };

   void setFirewallPolicy(const unsigned char firepolicy);

   const resip::NameAddr& getDialerIdentity() const;
   const resip::Uri& getRegIdentity() const;

   const resip::Data& getAuthUser() const;
   const resip::Data& getAuthRealm() const;
   const resip::Data& getAuthPassword() const;
   const resip::Data& getRTPPort() const;

#ifdef VIDEO_ENABLE
   void setVideoCapture(const bool status);
   bool getVideoCapture() const;
   void setVideoPlay(const bool status);
   bool getVideoPlay() const;

   void setVideoRTPPort(const resip::Data& locPort);
   const resip::Data& getVideoRTPPort() const;

   void setCodecH263_1998(const bool status);
   bool getCodecH263_1998() const;
#endif

   const resip::Data& getLocRealm() const;

   bool getCodecPCMU() const;
   bool getCodecPCMA() const;
   bool getCodecGSM() const;
   bool getCodecG723()  const;
   bool getCodecG729() const;
   bool getCodeciLBC() const;
   int  getCodecPriority() const;

   const resip::Data& getStunServer() const;
   const resip::Data& getStunPort() const;
   const resip::Data& getOutboundProxy() const;
   const resip::Data& getOutboundPort() const;
   DialerConfiguration::FirewallPolicy getFirewallPolicy() const;
   int getSendRegister() const;

   const resip::Data& getPlayFile() const;
   const resip::Data& getRecordFile() const;
   const resip::Data& CurrentPath() const;

   int getOutAudioDev() const;
   int getInAudioDev() const;

   void setInbandDTMF(bool dtmf);
   bool getInbandDTMF() const;

   void setMicBoost(bool boost);
   bool getMicBoost() const;
   bool IsMicBoostEnable() const;

   void setAGCLevel(int nLevel);
   int getAGCLevel() const;

   void setEchoNoiseCancellation(bool echoc);
   bool getEchoNoiseCancellation() const;

   void setDonotDisturb(bool notdisturb);
   bool getDonotDisturb() const;

   std::list<resip::SdpContents::Session::Codec> mCodecs;

protected:
	
   // Credentials we must send if challenged in the given realm
   // Todo: allow a hashmap of credentials for multiple realms
   resip::Data mAuthUser;//name
   resip::Data mAuthPassword;//password
   resip::Data mLocRealm;
   resip::Data mListenPort;

   resip::Data mAuthRealm;//proxy
   resip::Data mDisplayName;
   resip::Data mOutboundProxy;
   resip::Data mOutboundPort;
   resip::Data mStunServer;
   resip::Data mStunPort;
   
   resip::Data mRtpPort;

   int mOutAudioDev;
   int mInAudioDev;

#ifdef VIDEO_ENABLE
   bool mVideoCapture;
   bool mVideoPlay;
   resip::Data mVideoRtpPort;
   
   bool mH263_1998;
#endif

   bool mPCMA;
   bool mPCMU;
   bool mGSM;
   bool mG723;
   bool mG729;
   bool mSpeex;
   bool miLBC;
   int  mCodecPrio;

   int mSendRegister;/**/
   resip::Data mPlayFile;
   resip::Data mRecordFile;

   bool mInbandDTMF;
   bool mMicBoost;
   int mAGCLevel;
   bool mEchoNoiseCancel;
   bool mDonotDisturb;

   FirewallPolicy mFirewallPolicy;/*0-NO FIREWALL; 1-USE OUTBOUND PROXY; 2-USE STUN*/
   resip::Data mCurrentPath;

   // Used for the `From' field of the INVITE
   resip::NameAddr mDialerIdentity;//displayname+string("<sip:")+name+string("@")+locip+string(">")
   resip::Uri mRegIdentity;//string(sip:")+name+string("@")+proxy
   resip::Uri mStunIdentity;
 
};


#endif




