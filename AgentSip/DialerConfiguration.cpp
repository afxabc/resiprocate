
#include <iostream>
#include <string>
#ifdef _WIN32
#include <winsock2.h>
#endif
#include "rutil/Data.hxx"
#include "DialerConfiguration.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace resip;
using namespace std;

DialerConfiguration::DialerConfiguration():
mSendRegister(-1),
mListenPort("5060"),
mRtpPort("8000"),
#ifdef VIDEO_ENABLE
mVideoCapture(false),
mVideoPlay(false),
mVideoRtpPort("9078"),
mH263_1998(true),
#endif
mStunPort("3478"),
mInbandDTMF(true),
mMicBoost(false),
mAGCLevel(-1),
mEchoNoiseCancel(true),
mDonotDisturb(false),
mPCMA(true),
mPCMU(true),
mGSM(true),
mG723(true),
mG729(true),
mSpeex(true),
miLBC(true),
mCodecPrio(18),
mOutAudioDev(0),
mInAudioDev(0)
{
	mFirewallPolicy=NO_FIREWALL;
	
	char buf[MAX_PATH];
	int len=GetCurrentDirectoryA(MAX_PATH,buf);
	if(len>0)
	{
		mCurrentPath=buf;
	}
	mPlayFile=mCurrentPath;
}

DialerConfiguration::~DialerConfiguration()
{
}

Data DialerConfiguration::getHostAddress()
{
   // if you change this, please #def old version for windows 
   char hostName[1024];
   int err =  gethostname( hostName, sizeof(hostName) );
   assert( err == 0 );
   
   struct hostent* hostEnt = gethostbyname( hostName );
   assert( hostEnt );
   
   struct in_addr* addr = (struct in_addr*) hostEnt->h_addr_list[0];
   assert( addr );
   
   // if you change this, please #def old version for windows 
   char* addrA = inet_ntoa( *addr );
   Data ret(addrA);

   return ret;
}

void DialerConfiguration::loadConfigure()
{
	if(mLocRealm==Data::Empty)
	{
		mLocRealm=getHostAddress();
	}
	Data data=mDisplayName;
	data+="<sip:";
	data+=mAuthUser;
	data+="@";
	data+=mAuthRealm;
	data+=">";
	mDialerIdentity=NameAddr(data);

	data="sip:";
	data+=mAuthUser;
	data+="@";
	data+=mAuthRealm;
	mRegIdentity=Uri(data);

	data="sip:";
	data+=mStunServer;
	data+=":";
	data+=mStunPort;
	mStunIdentity=Uri(data);

}

void DialerConfiguration::loadCodecConfig()
{
	mCodecs.clear();
	if(mPCMA)
	{
		mCodecs.push_back(SdpContents::Session::Codec::ALaw_8000);
	}
	if(mPCMU)
	{
		mCodecs.push_back(SdpContents::Session::Codec::ULaw_8000);
	}
	if(mG729)
	{
		mCodecs.push_back(SdpContents::Session::Codec::G729_8000);
	}
	if(mG723)
	{
		mCodecs.push_back(SdpContents::Session::Codec::G723_8000);
	}
	if(mGSM)
	{
		mCodecs.push_back(SdpContents::Session::Codec::GSM_8000);
	}
	if(miLBC)
	{
		mCodecs.push_back(SdpContents::Session::Codec::iLBC_8000);
	}
}

void DialerConfiguration::setFirewallPolicy(const unsigned char firepolicy) 
{
	switch(firepolicy)
	{
	case 0:
		mFirewallPolicy=NO_FIREWALL;
		break;
	case 1:
		mFirewallPolicy=USE_NAT_ADDRESS;
		break;
	case 2:
		mFirewallPolicy=USE_STUN;
		break;
	default:
		assert(0);
		break;
	}
}

void DialerConfiguration::loadStream(std::istream& in)
{
#if 0
   while(!in.eof())
   {
      string param;
      string value;
      in >> param >> value;
      cerr << "param = " << param << " and value = " << value << endl;
      if(param == string("dialerIdentity"))
	  {
         setDialerIdentity(NameAddr(Uri(Data(value))));
	  }
	  else if(param == string("regIdentity"))
	  {
         setRegIdentity(Uri(Data(value)));
	  }
      else if(param == string("authRealm"))
	  {
         setAuthRealm(Data(value));
	  }
      else if(param == string("authUser"))
	  {
         setAuthUser(Data(value));
	  }
      else if(param == string("authPassword"))
	  {
         setAuthPassword(Data(value));
	  }
      else if(param == string("callerUserAgentAddress"))
	  {
         setCallerUserAgentAddress(Uri(Data(value)));
	  }
      else if(param == string("localPort"))
	  {
         setLocalPort(Data(value));
	  }
	  else if(param == string("PCMA"))
	  {
         setCodecPCMA(Data(value));
		 SdpContents::Session::Codec codec1("PCMA", 8, 8000);
		 mCodecs.push_back(codec1);
	  }
	  else if(param == string("PCMU"))
	  {
         setCodecPCMU(Data(value));
		 SdpContents::Session::Codec codec2("PCMU", 0, 8000);
		 mCodecs.push_back(codec2);
	  }
	  else if(param == string("GSM"))
	  {
         setCodecGSM(Data(value));
		 SdpContents::Session::Codec codec3("GSM", 3, 8000);
		 mCodecs.push_back(codec3);
	  }
	  else if(param == string("iLBC"))
	  {
         setCodeciLBC(Data(value));
		 SdpContents::Session::Codec codec4("iLBC", 102, 8000);
		 mCodecs.push_back(codec4);
	  }
	  else if(param == string(""))
         // skip empty lines
         { }
	  else
	  {
         assert(0); // FIXME
	  }
   }
#endif
}

void DialerConfiguration::saveStream(std::ostream& out)
{
#if 0
   {
	   out<<string("dialerIdentity")<<" "<<mDialerIdentity.displayName()<<mDialerIdentity.uri()<<">"<<endl;
	   out<<string("regIdentity")<<" "<<mRegIdentity<<endl;
	   out<<string("authRealm")<<" "<<mAuthRealm<<endl;
	   out<<string("authUser")<<" "<<mAuthUser<<endl;
	   out<<string("authPassword")<<" "<<mAuthPassword<<endl;
	   
	   out<<string("PCMU")<<" "<<mPCMU<<endl;
	   out<<string("PCMA")<<" "<<mPCMA<<endl;
	   out<<string("callerUserAgentAddress")<<" "<<mCallerUserAgentAddress<<endl;

   }
#endif
}

const resip::NameAddr& DialerConfiguration::getDialerIdentity() const
{ 
	return mDialerIdentity; 
};

const resip::Uri& DialerConfiguration::getRegIdentity() const
{ 
	return mRegIdentity;
};

const resip::Data& DialerConfiguration::getAuthUser() const
{ 
	return mAuthUser;
};
   
const resip::Data& DialerConfiguration::getAuthRealm() const
{ 
	return mAuthRealm; 
};
   
const resip::Data& DialerConfiguration::getAuthPassword() const
{ 
	return mAuthPassword;
};
   
const resip::Data& DialerConfiguration::getRTPPort() const
{ 
	return mRtpPort;
};

#ifdef VIDEO_ENABLE
  
void DialerConfiguration::setVideoCapture(const bool status)
{
	mVideoCapture = status; 
};
   
bool DialerConfiguration::getVideoCapture()  const
{ 
	return mVideoCapture;
};
  
void DialerConfiguration::setVideoPlay(const bool status)
{
	mVideoPlay = status;
};
  
bool DialerConfiguration::getVideoPlay() const
{ 
	return mVideoPlay;
};

  
void DialerConfiguration::setVideoRTPPort(const resip::Data& locPort)
{ 
	mVideoRtpPort = locPort;
};
   
const resip::Data& DialerConfiguration::getVideoRTPPort() const
{ 
	return mVideoRtpPort;
};

  
void DialerConfiguration::setCodecH263_1998(const bool status)
{
	mH263_1998 = status;
};
  
bool DialerConfiguration::getCodecH263_1998() const
{
	return mH263_1998;
};

#endif

   
const resip::Data& DialerConfiguration::getLocRealm() const
{
	return mLocRealm;
};

  
bool DialerConfiguration::getCodecPCMU() const 
{
	return mPCMU;
};
  
bool DialerConfiguration::getCodecPCMA() const
{ 
	return mPCMA;
};
  
bool DialerConfiguration::getCodecGSM() const
{
	return mGSM;
};
  
bool DialerConfiguration::getCodecG723() const
{ 
	return mG723;
};
  
bool DialerConfiguration::getCodecG729() const
{ 
	return mG729;
};
  
bool DialerConfiguration::getCodeciLBC() const
{ 
	return miLBC;
};
  
int  DialerConfiguration::getCodecPriority() const
{
	return mCodecPrio;
};

  
const resip::Data& DialerConfiguration::getStunServer() const
{
	return mStunServer;
};
  
const resip::Data& DialerConfiguration::getStunPort() const
{
	return mStunPort;
};
  
const resip::Data& DialerConfiguration::getOutboundProxy() const
{
	return mOutboundProxy;
};
   
const resip::Data& DialerConfiguration::getOutboundPort() const
{
	return mOutboundPort;
}

DialerConfiguration::FirewallPolicy DialerConfiguration::getFirewallPolicy() const
{
	return mFirewallPolicy;
};
   
int DialerConfiguration::getSendRegister() const
{
	return mSendRegister;
};

   
const resip::Data& DialerConfiguration::getPlayFile() const
{
	return mPlayFile; 
};
   
const resip::Data& DialerConfiguration::getRecordFile() const
{
	return mRecordFile;
};
  
const resip::Data& DialerConfiguration::CurrentPath() const
{ 
	return mCurrentPath;
};

int DialerConfiguration::getOutAudioDev() const
{
	return mOutAudioDev;
}

int DialerConfiguration::getInAudioDev() const
{ 
	return mInAudioDev;
}

void DialerConfiguration::setInbandDTMF(bool dtmf)
{
	mInbandDTMF=dtmf;
};
   
bool DialerConfiguration::getInbandDTMF() const
{
	return mInbandDTMF;
};

  
void DialerConfiguration::setMicBoost(bool boost)
{
	mMicBoost=boost;
};
   
bool DialerConfiguration::getMicBoost() const
{
	return mMicBoost;
};
  
bool DialerConfiguration::IsMicBoostEnable() const
{
	return (mMicBoost==true);
};

   
void DialerConfiguration::setAGCLevel(int nLevel)
{
	mAGCLevel=nLevel;
};
  
int DialerConfiguration::getAGCLevel() const
{
	return mAGCLevel;
};

  
void DialerConfiguration::setEchoNoiseCancellation(bool echoc)
{
	mEchoNoiseCancel=echoc;
};
   
bool DialerConfiguration::getEchoNoiseCancellation() const
{
	return mEchoNoiseCancel;
};

  
void DialerConfiguration::setDonotDisturb(bool notdisturb)
{
	mDonotDisturb=notdisturb;
};
  
bool DialerConfiguration::getDonotDisturb() const
{
	return mDonotDisturb;
};

/* ====================================================================
 * The Vovida Software License, Version 1.0
 *
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * ====================================================================
 *
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

