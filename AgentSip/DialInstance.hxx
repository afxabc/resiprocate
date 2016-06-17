
#ifndef __DIALINSTANCE_H
#define __DIALINSTANCE_H

#include <string>
#include <map>
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/ClientInviteSession.hxx"
#include "resip/dum/InviteSessionHandler.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/stack/Uri.hxx"
#include "rutil/Data.hxx"

#include "DialerConfiguration.hxx"
#include "MyAudioStream.hxx"
#ifdef VIDEO_ENABLE
#include "MyVideoStream.hxx"
#endif
#include "mediastream/MSSndCard.h"
#include "PhoneLine.hxx"
#include "TaskManager.hxx"
#include "msticker.h"

class MyInviteSessionHandler;
class MySipDialer;
class AppSession;
class AudioConf;

class DialInstance : public TaskManager::RecurringTask
{

public:
   DialInstance(DialerConfiguration& dialerConfiguration);
   virtual ~DialInstance();
   
   TaskManager::TaskResult doTaskProcessing();

  /**
   * Stop accepting new calls 
   * Shutdown existing calls
   * Blocks until all existing calls stopped
   */
  void stop();
  bool isStopping();


   typedef enum 
   {
      ReferSuccessful,
      ReferUnsuccessful,
      Error
   } DialResult;
   DialResult execute();
   
   bool OpenLine(int nLineNo, bool bBindToRTPRxIP, resip::Data sRTPRxIP, unsigned int nRTPRxPort);
   bool CloseLine(int nLineNo);
   void setSipStack(resip::SipStack *stack);
   void setDum(resip::DialogUsageManager *dum);
   resip::DialogUsageManager * getDum();

   AppSession* getCallSession(InviteSession *is);
   bool sendInvite(int nLineNo, resip::Data sToURI, int nInputDevice, int nOutputDevice);
   void sendAnswer(int nLineNo, resip::Data sCallId, int nInputDeviceId, int nOutputDeviceId);
   void sendReject(resip::Data sCallId);
   void sendHangup(int nLineNo);

   bool HoldLine(int nLineNo);
   bool UnHoldLine(int nLineNo);

   bool Transfer(int nLineNo, resip::Data sToURI);
   
   bool LineIsOpen(int nLineNo);
   bool LineIsHold(int nLineNo);
   bool LineIsBusy(int nLineNo);

   bool DigitDTMF(int nLineNo, resip::Data sDigit);
   bool SetDTMFVolume(int nVolume);
   int GetDTMFVolume();
   bool EnableInbandDTMF(int nLineNo,bool dtmf);

   int GetMicSoundLevel();
   int GetSpkSoundLevel();
   bool MuteMic(bool bMute);
   bool MuteSpk(bool bMute);
   int GetMicVolume();
   bool SetMicVolume(int nVolume);
   int GetSpkVolume();
   bool SetSpkVolume(int nVolume);

   void setOutAudioDevice(int val);
   void setEchoNoiseCancellation(bool ec);
   void setAGCLevel(int level);
   void setMicBoost(bool boost);

   bool IsRecording(int nLineNo);
   bool StartRecording(int nLineNo, int nRecordVoice, bool bRecordCompress);
   bool StopRecording(int nLineNo);
   bool ResetRecording(int nLineNo);
   bool SaveRecordingToWaveFile(int nLineNo, resip::Data sFileName);

   bool IsWaveFilePlaying(int nLineNo);
   bool PlayWaveOpen(int nLineNo, resip::Data sFileName);
   bool PlayWaveSkipTo(int nLineNo, int nSeconds);
   int PlayWaveTotalTime(int nLineNo);
   bool PlayWavePause(int nLineNo);
   bool PlayWaveStart(int nLineNo, bool bListen);
   bool PlayWaveStop(int nLineNo);
   int PlayWavePosition(int nLineNo);

   bool SetTOS(int nLineNo, int nValue);
   int GetTOS(int nLineNo);

   int GetOutboundCodec(int nLineNo);
   int GetInboundCodec(int nLineNo);

   void msgNOTIFY(resip::Data sMsg) ;

   void msgConnecting(int nLineNo);
   void msgSuccessToConnect(int nLineNo, resip::Data sToRTPIP, int nToRTPPort);
   void msgFailToConnect(int nLineNo);
   
   void msgDisconnectCall(int nLineNo);
   void msgCallTransferAccepted(int nLineNo);
   void msgPlayWaveDone(int nLineNo);
   void msgDTMFDigit(int nLineNo, resip::Data sDigit);
   
   void msgVoiceMailMsg(bool bIsMsgWaiting, 
	   unsigned long dwNewMsgCount, 
	   unsigned long dwOldMsgCount, 
	   unsigned long dwNewUrgentMsgCount, 
	   unsigned long dwOldUrgentMsgCount, 
	   resip::Data sMsgAccount);
   
	void msgIncomingCall(resip::Data sCallId, resip::Data sDisplayName, resip::Data sUserName, resip::Data sFromURI, resip::Data sToURI);
	void msgIncomingCallRingingStart(resip::Data sCallId);
	void msgIncomingCallRingingStop(resip::Data sCallId);
	void msgIncomingCallMissed(resip::Data sCallId);
	void msgHoldCall(int nLineNo);
	void msgTransferStart(int nLineNo, resip::Data sUri);
	void msgTransferSuccess(int nLineNo);
	void msgTransferFail(int nLineNo);

	void msgIncomingDiagnostic(resip::Data sMsgSIP, resip::Data sFromIP, unsigned int nFromPort);
	void msgOutgoingDiagnostic(resip::Data sMsgSIP, resip::Data sToIP, unsigned int nToPort);

   void msgProvisionalResponse(int nLineNo, int nStatusCode, resip::Data sReasonPhrase);
   void msgRedirectionResponse(int nLineNo, int nStatusCode, resip::Data sReasonPhrase, resip::Data sContact);
   void msgRequestFailureResponse(int nLineNo, int nStatusCode, resip::Data sReasonPhrase);
   void msgServerFailureResponse(int nLineNo, int nStatusCode, resip::Data sReasonPhrase);
   void msgGeneralFailureResponse(int nLineNo, int nStatusCode, resip::Data sReasonPhrase);

   void msgStartPaint();
   void msgStopPaint();
   void msgPaintBuf(int fmt, int w, int h, uint8_t* buf, int len);

   void prepareAddress();
   
   resip::Data processNumber(const resip::Data& verboseNumber);

   // Receive notifications from MyInviteSessionHandler
   
   // some kind of failure message (4xx, 5xx, 6xx) received
   void onFailure(resip::ClientInviteSessionHandle cis, const resip::SipMessage& msg);
   // the session has connected and is ready for a REFER
   void onConnected(resip::ClientInviteSessionHandle cis, const resip::SipMessage& msg);
   void onNewSession(resip::ServerInviteSessionHandle sis, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
   void onNewSession(resip::ClientInviteSessionHandle cis, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
   void onConnected(InviteSessionHandle is, const resip::SipMessage& msg);
   
   // the REFER succeeded
   void onRefer(resip::InviteSessionHandle is, resip::ServerSubscriptionHandle ssh, const resip::SipMessage& msg);
   void onReferAccepted(InviteSessionHandle is, ClientSubscriptionHandle csh, const SipMessage& msg);
   // the REFER failed
   void onReferRejected(InviteSessionHandle is, const SipMessage& msg);

   void onEarlyMedia(resip::ClientInviteSessionHandle cis, const resip::SipMessage& msg, const resip::SdpContents& sdp);
   void onProvisional(resip::ClientInviteSessionHandle cis, const resip::SipMessage& msg);
   
   void onOffer(resip::InviteSessionHandle is,const resip::SipMessage& msg, const resip::SdpContents& sdp);
   void onOfferRequired(resip::InviteSessionHandle sis, const resip::SipMessage& msg);
   void onOfferRejected(InviteSessionHandle is, const SipMessage *msg);
   void onAnswer(resip::InviteSessionHandle is, const resip::SipMessage& msg, const resip::SdpContents& sdp);
   // the session has been terminated
   void onTerminated(resip::InviteSessionHandle is, resip::InviteSessionHandler::TerminatedReason reason, const resip::SipMessage* msg);
   void addAppSession(resip::Data cid, AppSession* session);
   void addAppSession(int cid, AppSession* session);

   AppSession* findAppSession(const int cid);
   AppSession* findAppSession(const resip::Data cid);
   std::vector<AppSession*> findAppSessionList(const int cid);

   void eraseAppSession(resip::Data cid);
   void eraseAppSession(AppSession* is);

   void addLine(int id, PhoneLine* line);
   PhoneLine* findLine(int id);
   void eraseLine(int id);

   const DialerConfiguration& getDialerConfig() {return mDialerConfiguration;}

   MySipDialer *getMySipDialer() const {return mSipDialer;};
   void setMySipDialer(MySipDialer *dialer) {mSipDialer=dialer;};

   MSSndCard* getMSSndCard() const {return mscard;};
   MSTicker *getAudioTicker() const {return mAudioTick; };
   AudioConf *getAudioConf() const {return mConf; };

private:
   // Copy of values supplied when instance created
   DialerConfiguration& mDialerConfiguration;
   resip::Uri mTargetUri;
   // The target URI, converted to a sip: URI if necessary
   resip::Uri mFullTarget;

   resip::SipStack *mSipStack;
   resip::DialogUsageManager *mDum;

   MyInviteSessionHandler *ish;
   MSSndCard *mscard;

   DialResult mResult;

   bool stopping;
   bool mustStopCalls;
	
   std::list<AppSession*> mCallList;
   
   std::map<int,PhoneLine*> mLineMap;

   mutable resip::Mutex mMut;

   MSTicker *mAudioTick;

   AudioConf *mConf;

   MySipDialer *mSipDialer;

   // MyInviteSessionHandler will notify us of progress
//   friend class MyInviteSessionHandler;
};

#endif


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

