


#include "resip/stack/ExtensionHeader.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/stack/Tuple.hxx"
#include "rutil/Data.hxx"

#include "DialInstance.hxx"
#include "MyInviteSessionHandler.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace resip;
using namespace std;

MyInviteSessionHandler::MyInviteSessionHandler(DialInstance& dialInstance) : 
   mDialInstance(dialInstance)
{
}

MyInviteSessionHandler::~MyInviteSessionHandler()
{
}

void MyInviteSessionHandler::onSuccess(ClientRegistrationHandle h, const SipMessage& response) 
{
	InfoLog(<<"MyInviteSessionHandler::onSuccess");
}

void MyInviteSessionHandler::onFailure(ClientRegistrationHandle, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onFailure");
}

void MyInviteSessionHandler::onMessage(InviteSessionHandle, const resip::SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onMessage");
}

void MyInviteSessionHandler::onMessageSuccess(InviteSessionHandle, const resip::SipMessage&) 
{
	InfoLog(<<"MyInviteSessionHandler::onMessageSuccess");
}

void MyInviteSessionHandler::onMessageFailure(InviteSessionHandle, const resip::SipMessage&) 
{
	InfoLog(<<"MyInviteSessionHandler::onMessageFailure");
}

void MyInviteSessionHandler::onFailure(ClientInviteSessionHandle cis, const SipMessage& msg) 
{
   mDialInstance.onFailure(cis, msg);
   InfoLog(<<"MyInviteSessionHandler::onFailure");
}

void MyInviteSessionHandler::onForkDestroyed(ClientInviteSessionHandle) 
{
	InfoLog(<<"MyInviteSessionHandler::onForkDestroyed");
}

void MyInviteSessionHandler::onInfoSuccess(InviteSessionHandle, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onInfoSuccess");
}

void MyInviteSessionHandler::onInfoFailure(InviteSessionHandle, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onInfoFailure");
}

void MyInviteSessionHandler::onProvisional(ClientInviteSessionHandle cis, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onProvisional");
	mDialInstance.onProvisional(cis,msg);
}

void MyInviteSessionHandler::onConnected(ClientInviteSessionHandle cis, const SipMessage& msg) 
{
   mDialInstance.onConnected(cis,msg);

   SdpContents *sdp = (SdpContents*)msg.getContents();
//   cis->provideAnswer(*sdp);
   InfoLog(<<"MyInviteSessionHandler::onConnected");
}

void MyInviteSessionHandler::onStaleCallTimeout(ClientInviteSessionHandle) 
{
	InfoLog(<<"MyInviteSessionHandler::onStaleCallTimeout");
}

void MyInviteSessionHandler::onConnected(InviteSessionHandle is, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onConnected");
	mDialInstance.onConnected(is,msg);

   SdpContents *sdp = (SdpContents*)msg.getContents();
//   is->provideAnswer(*sdp);
}

void MyInviteSessionHandler::onRedirected(ClientInviteSessionHandle, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onRedirected");
}

void MyInviteSessionHandler::onAnswer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp) 
{
	InfoLog(<<"MyInviteSessionHandler::onAnswer");
	mDialInstance.onAnswer(is,msg,sdp);
}

void MyInviteSessionHandler::onEarlyMedia(ClientInviteSessionHandle cis, const SipMessage& msg, const SdpContents& sdp) 
{
	InfoLog(<<"MyInviteSessionHandler::onEarlyMedia");
	mDialInstance.onEarlyMedia(cis, msg, sdp);
}

void MyInviteSessionHandler::onOfferRequired(InviteSessionHandle is, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onOfferRequired");
	mDialInstance.onOfferRequired(is, msg);
}

void MyInviteSessionHandler::onOfferRejected(InviteSessionHandle is, const SipMessage *msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onOfferRejected");
	mDialInstance.onOfferRejected(is, msg);
}

void MyInviteSessionHandler::onDialogModified(InviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onDialogModified");
}

void MyInviteSessionHandler::onInfo(InviteSessionHandle, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onInfo");
}

void MyInviteSessionHandler::onRefer(InviteSessionHandle is, ServerSubscriptionHandle ssh, const SipMessage& msg) 
{
	mDialInstance.onRefer(is, ssh, msg);
	InfoLog(<<"MyInviteSessionHandler::onRefer");
}

void MyInviteSessionHandler::onReferAccepted(InviteSessionHandle is, ClientSubscriptionHandle csh, const SipMessage& msg) 
{
   mDialInstance.onReferAccepted(is, csh,  msg);
   InfoLog(<<"MyInviteSessionHandler::onReferAccepted");
}

void MyInviteSessionHandler::onReferRejected(InviteSessionHandle is, const SipMessage& msg) 
{
   mDialInstance.onReferRejected(is, msg);
   InfoLog(<<"MyInviteSessionHandler::onReferRejected");
}

void MyInviteSessionHandler::onReferNoSub(InviteSessionHandle is, const resip::SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onReferNoSub");
}

void MyInviteSessionHandler::onRemoved(ClientRegistrationHandle) 
{
	InfoLog(<<"MyInviteSessionHandler::onRemoved");
}

int MyInviteSessionHandler::onRequestRetry(ClientRegistrationHandle, int retrySeconds, const SipMessage& response) 
{
  return -1;
}

void MyInviteSessionHandler::onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onNewSession");
	mSis = sis;
//	sis->provisional(180);
	mDialInstance.onNewSession(sis,oat,msg);
}

void MyInviteSessionHandler::onNewSession(ClientInviteSessionHandle cis, InviteSession::OfferAnswerType oat, const SipMessage& msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onNewSession");
	mDialInstance.onNewSession(cis, oat, msg);
}

void MyInviteSessionHandler::onTerminated(InviteSessionHandle is, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg) 
{
	InfoLog(<<"MyInviteSessionHandler::onTerminated");
   mDialInstance.onTerminated(is, reason, msg);
}

void MyInviteSessionHandler::onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp) 
{
	InfoLog(<<"MyInviteSessionHandler::onOffer");
	mDialInstance.onOffer(is, msg, sdp);
//	is->provideAnswer(sdp);
}


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

