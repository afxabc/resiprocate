

#include "resip/dum/ClientAuthManager.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/stack/SipStack.hxx"
#include "resip/stack/Uri.hxx"
#include "resip/stack/UdpTransport.hxx"
#include "rutil/Data.hxx"
#include "rutil/SharedPtr.hxx"

#include "mediastream/WincardWrite.h"
#include "DialerConfiguration.hxx"
#include "DialInstance.hxx"
#include "MyInviteSessionHandler.hxx"
#include "MySipDialer.hxx"
#include "MyAppDialogSet.hxx"
#include "DialInstance.hxx"
#include "AppSession.hxx"
#include "mediastream/audioconf.h"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace resip;
using namespace std;

DialInstance::DialInstance(DialerConfiguration& dialerConfiguration) :
   mDialerConfiguration(dialerConfiguration),
   mResult(Error),
   mustStopCalls(false),
   stopping(false),
   mscard(new MSSndCard()),
   ish(NULL),
   mAudioTick(new MSTicker()),
   mConf(new AudioConf())
{
}

DialInstance::~DialInstance()
{
/*	std::list<AppSession*>::iterator iter1=mCallList.begin();
	while(iter1!=mCallList.end())
	{
		AppSession* tmp=(*iter1);
		iter1++;
		mCallList.remove(tmp);
		delete tmp;
	}*/
	std::map<int,PhoneLine*>::iterator iter2 = mLineMap.begin();
	for(; iter2 != mLineMap.end(); ++iter2)
	{
		delete iter2->second;
	}
	if(ish)
	{
		delete ish;
	}
//	mLineMap.clear();
	delete mscard;
	delete mAudioTick;
	delete mConf;
}

TaskManager::TaskResult 
DialInstance::doTaskProcessing()
{
  time_t now;

  Lock lock(mMut); (void)lock;

  if(mustStopCalls)
  {
    InfoLog(<<"notifying calls to stop");
    list<AppSession *>::iterator call = mCallList.begin();
    while(call != mCallList.end()) {
      (*call)->onStopping();
      call++;
    }
    mustStopCalls = false;
  }

  time(&now);
  list<AppSession *>::iterator i = mCallList.begin();
  while(i != mCallList.end())
  {
    (*i)->checkProgress(now, stopping);
    if((*i)->isComplete())
	{
      AppSession *call = *i;
      i++;
      mCallList.remove(call);
      delete call;
    }
	else
	{
      i++;
	}
  }
  if(stopping && mCallList.begin() == mCallList.end())
  {
    InfoLog(<<"no (more) calls in progress");
    return TaskManager::TaskComplete;
  }

  mscard->process();
  return TaskManager::TaskNotComplete;
}

void 
DialInstance::stop()
{
  stopping = true;
  mustStopCalls = true;
}

bool 
DialInstance::isStopping()
{
  return stopping;
}


void
DialInstance::setSipStack(SipStack *stack)
{
	mSipStack=stack;
}

void
DialInstance::setDum(DialogUsageManager *dum)
{
	mDum=dum;
}

resip::DialogUsageManager *
DialInstance::getDum()
{
	return mDum;
}


DialInstance::DialResult
DialInstance::execute()
{
   ish = new MyInviteSessionHandler(*this);
   mDum->setInviteSessionHandler(ish);

   return mResult;
}

void
DialInstance::prepareAddress() 
{
   if(mTargetUri.scheme() == Symbols::Sip) {
      mFullTarget = mTargetUri;
      return;
   }

   if(mTargetUri.scheme() == Symbols::Tel) {
      Data num = processNumber(mTargetUri.user());
      if(num.size() < 1)
      {
         // FIXME - check size
         assert(0);
      }
      if(num[0] == '+')
      {
         // E.164
//         mFullTarget = Uri("sip:" + mDialerConfiguration.getTargetPrefix() + num.substr(1, num.size() - 1) + "@" + mDialerConfiguration.getTargetDomain());
         return;
      }
//      mFullTarget = Uri("sip:" + num + "@" + mDialerConfiguration.getTargetDomain());
      return;
   }

   // FIXME Unsupported scheme 
   assert(0);
}

bool 
DialInstance::OpenLine(int nLineNo, bool bBindToRTPRxIP, Data sRTPRxIP, unsigned int nRTPRxPort)
{
	PhoneLine *line=new PhoneLine(nLineNo);
	if(bBindToRTPRxIP)
	{
		line->setRTPRxIP(sRTPRxIP);
		line->setRTPRxPort(nRTPRxPort);
	}
	addLine(nLineNo,line);
	return true;
}

bool DialInstance::CloseLine(int nLineNo)
{
	eraseLine(nLineNo);
	return true;
}

AppSession* 
DialInstance::getCallSession(InviteSession *is)
{
	MyAppDialog *myAppDialog = (MyAppDialog *)is->getAppDialog().get();
	return (AppSession *)myAppDialog->getCallSession();
}

bool
DialInstance::sendInvite(int nLineNo, resip::Data sToURI, int nInputDevice, int nOutputDevice) 
{
	
	PhoneLine* line=findLine(nLineNo);
	if(!line)
	{
		msgNOTIFY("line is not open");
		return true;
	}
	if(line->getLineState())
	{
		msgNOTIFY("line is busy");
		return true;
	}

	msgNOTIFY(Data("call ")+sToURI);
	int pos=sToURI.find("sip:");
	if(pos<0)
	{
		sToURI=Data("sip:")+sToURI+Data("@")+mDialerConfiguration.getAuthRealm();
	}

	AppSession *call = new AppSession(this, mDum, mDialerConfiguration.getDialerIdentity(), Uri(sToURI), line,nInputDevice, nOutputDevice);
	
	msgConnecting(line->getLineNo());

	Lock lock(mMut); (void)lock;
	mCallList.push_back(call);

	AppSession::AppMessage *msg = new AppSession::AppMessage(AppSession::AppMessage::OutCallMsg);
	call->mMessageList.push_back(msg);
	
//	call->doOutgoingCall();
	
	return true;
//	delete sdp;
}

void
DialInstance::sendAnswer(int nLineNo, resip::Data sCallId, int nInputDeviceId, int nOutputDeviceId)
{
	AppSession* session = findAppSession(sCallId);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return;
	}
	PhoneLine* line=findLine(nLineNo);
	if(!line)
	{
		msgNOTIFY("line is not open");
		return;
	}
	if(line->getLineState())
	{
		msgNOTIFY("line is busy");
		return;
	}
	
	Lock lock(mMut); (void)lock;
	session->setPhoneLine(line);

	AppSession::AppMessage *msg = new AppSession::AppMessage(AppSession::AppMessage::SendAnswerMsg, 
		nLineNo, sCallId, Data::Empty, 
		nInputDeviceId, nOutputDeviceId);
	session->mMessageList.push_back(msg);

//	session->CallAccept(line, nInputDeviceId, nOutputDeviceId);

//	delete sdp;
}

void
DialInstance::sendReject(resip::Data sCallId)
{
	AppSession* session = findAppSession(sCallId);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return;
	}

	Lock lock(mMut); (void)lock;
	AppSession::AppMessage *msg = new AppSession::AppMessage(AppSession::AppMessage::SendRejectMsg, 
		0, sCallId);
	session->mMessageList.push_back(msg);

//	session->CallReject();
}

void
DialInstance::sendHangup(int nLineNo)
{
	std::vector<AppSession*> sessions = findAppSessionList(nLineNo);
	if( sessions.empty() )
	{
		msgNOTIFY(Data("line doesn't exist"));
		return;
	}

	Lock lock(mMut); (void)lock;
	std::vector<AppSession*>::iterator it = sessions.begin();
	while( it != sessions.end() )
	{
		AppSession::AppMessage *msg = new AppSession::AppMessage(AppSession::AppMessage::SendHangupMsg, nLineNo);
		(*it)->mMessageList.push_back(msg);

//		(*it)->Disconnect();

		it++;
	}
}

bool DialInstance::HoldLine(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}

	Lock lock(mMut); (void)lock;
	AppSession::AppMessage *msg = new AppSession::AppMessage(AppSession::AppMessage::HoldLineMsg, nLineNo);
	session->mMessageList.push_back(msg);

	return true;//session->LineHold();
}

bool DialInstance::UnHoldLine(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}

	Lock lock(mMut); (void)lock;
	AppSession::AppMessage *msg = new AppSession::AppMessage(AppSession::AppMessage::UnholdLineMsg, nLineNo);
	session->mMessageList.push_back(msg);

	return true;//session->LineUnHold();
}

bool DialInstance::Transfer(int nLineNo, Data sToURI)
{
	msgNOTIFY(Data("Transfer ")+sToURI);
	int pos=sToURI.find("sip:");
	if(pos<0)
	{
		sToURI=Data("sip:")+sToURI+Data("@")+mDialerConfiguration.getAuthRealm();
	}
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}

	Lock lock(mMut); (void)lock;
	AppSession::AppMessage *msg = new AppSession::AppMessage(AppSession::AppMessage::TransferCallMsg, nLineNo, Data::Empty, sToURI);
	session->mMessageList.push_back(msg);

//	session->CallTransfer(sToURI);
	return true;
}

void DialInstance::msgNOTIFY(resip::Data sMsg)
{
	mSipDialer->OnMsgNOTIFY(sMsg.data());
}

void DialInstance::msgConnecting(int nLineNo) 
{
	mSipDialer->OnConnecting(nLineNo);
}

void DialInstance::msgSuccessToConnect(int nLineNo, resip::Data sToRTPIP, int nToRTPPort)
{
	mSipDialer->OnSuccessToConnect(nLineNo, sToRTPIP.data(), nToRTPPort);
}

void DialInstance::msgFailToConnect(int nLineNo) 
{
	mSipDialer->OnFailToConnect(nLineNo);
}
   
void DialInstance::msgDisconnectCall(int nLineNo)
{
	mSipDialer->OnDisconnectCall(nLineNo);
}

void DialInstance::msgCallTransferAccepted(int nLineNo)
{
	mSipDialer->OnCallTransferAccepted(nLineNo);
}

void DialInstance::msgPlayWaveDone(int nLineNo)
{
	mSipDialer->OnPlayWaveDone(nLineNo);
}

void DialInstance::msgDTMFDigit(int nLineNo, resip::Data sDigit)
{
	mSipDialer->OnDTMFDigit(nLineNo, sDigit.data());
}

void DialInstance::msgVoiceMailMsg(bool bIsMsgWaiting, 
	   unsigned long dwNewMsgCount, 
	   unsigned long dwOldMsgCount, 
	   unsigned long dwNewUrgentMsgCount, 
	   unsigned long dwOldUrgentMsgCount, 
	   resip::Data sMsgAccount)
{
	mSipDialer->OnVoiceMailMsg(bIsMsgWaiting, dwNewMsgCount, dwOldMsgCount,dwNewUrgentMsgCount,dwOldUrgentMsgCount,sMsgAccount.data());
}

void DialInstance::msgIncomingCall(resip::Data sCallId, 
					 resip::Data sDisplayName, 
					 resip::Data sUserName, 
					 resip::Data sFromURI, 
					 resip::Data sToURI)
{
	mSipDialer->OnIncomingCall(sCallId.data(),sDisplayName.data(), sUserName.data(), sFromURI.data(), sToURI.data());
}

void DialInstance::msgIncomingCallRingingStart(resip::Data sCallId) 
{
	mSipDialer->OnIncomingCallRingingStart(sCallId.data());
}

void DialInstance::msgIncomingCallRingingStop(resip::Data sCallId) 
{
	mSipDialer->OnIncomingCallRingingStop(sCallId.data());
}

void DialInstance::msgIncomingCallMissed(resip::Data sCallId) 
{
	mSipDialer->OnIncomingCallMissed(sCallId.data());
}

void DialInstance::msgHoldCall(int nLineNo)
{
	mSipDialer->OnHoldCallMessage(nLineNo);
}

void DialInstance::msgTransferStart(int nLineNo, resip::Data sUri)
{
	mSipDialer->OnCallTransferStart(nLineNo, std::string(sUri.c_str()));
}

void DialInstance::msgTransferSuccess(int nLineNo)
{
	mSipDialer->OnCallTransferSuccess(nLineNo);
}

void DialInstance::msgTransferFail(int nLineNo)
{
	mSipDialer->OnCallTransferFail(nLineNo);
}

void DialInstance::msgIncomingDiagnostic(resip::Data sMsgSIP, resip::Data sFromIP, unsigned int nFromPort)
{
	mSipDialer->OnIncomingDiagnostic(sMsgSIP.data(), sFromIP.data(), nFromPort);
}

void DialInstance::msgOutgoingDiagnostic(resip::Data sMsgSIP, resip::Data sToIP, unsigned int nToPort)
{
	mSipDialer->OnOutgoingDiagnostic(sMsgSIP.data(), sToIP.data(), nToPort);
}

void DialInstance::msgProvisionalResponse(int nLineNo, int nStatusCode, resip::Data sReasonPhrase)
{
	mSipDialer->OnProvisionalResponse(nLineNo, nStatusCode, sReasonPhrase.data());
}

void DialInstance::msgRedirectionResponse(int nLineNo, int nStatusCode, resip::Data sReasonPhrase, resip::Data sContact)
{
	mSipDialer->OnRedirectionResponse(nLineNo, nStatusCode, sReasonPhrase.data(), sContact.data());
}

void DialInstance::msgRequestFailureResponse(int nLineNo, int nStatusCode, resip::Data sReasonPhrase)
{
	mSipDialer->OnRequestFailureResponse(nLineNo, nStatusCode, sReasonPhrase.data());
}

void DialInstance::msgServerFailureResponse(int nLineNo, int nStatusCode, resip::Data sReasonPhrase)
{
	mSipDialer->OnServerFailureResponse(nLineNo, nStatusCode, sReasonPhrase.data());
}

void DialInstance::msgGeneralFailureResponse(int nLineNo, int nStatusCode, resip::Data sReasonPhrase)
{
	mSipDialer->OnGeneralFailureResponse(nLineNo, nStatusCode, sReasonPhrase.data());
}

void DialInstance::msgStartPaint()
{
	mSipDialer->OnVideoStartMessage();
}

void DialInstance::msgStopPaint()
{
	mSipDialer->OnVideoStopMessage();
}

void DialInstance::msgPaintBuf(int fmt, int w, int h, unsigned char* buf, int len)
{
	mSipDialer->OnVideoPaintMessage(fmt, w, h, buf, len);
}

void DialInstance::onFailure(ClientInviteSessionHandle cis, const SipMessage& msg)
{
	DebugLog(<<"onFailure: "<<msg.header(h_StatusLine).statusCode()<<msg.header(h_StatusLine).reason().c_str());
	AppSession *call = getCallSession(cis.get());
	if(call == NULL)
	{
		WarningLog(<<"onFailure: unrecognised dialog");
		return;
	}
	call->onFailure(cis, msg);
}

void DialInstance::onConnected(ClientInviteSessionHandle cis, const SipMessage& msg) 
{
	AppSession *call = getCallSession(cis.get());
	if(call == NULL)
	{
		WarningLog(<<"onConnected: unrecognised dialog");
		return;
	}
	
	call->onConnected(cis, msg);
	msgNOTIFY("connected ");
}

void DialInstance::onConnected(InviteSessionHandle is, const SipMessage& msg)
{
	AppSession *call = getCallSession(is.get());
	if(call == NULL)
	{
		WarningLog(<<"onConnected: unrecognised dialog");
		return;
	}
	
	call->onConnected(is, msg);
	msgNOTIFY("connected ");
}

void DialInstance::onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg) 
{
	if(mDialerConfiguration.getDonotDisturb())
	{
		msgNOTIFY(Data("new incoming call from ")+msg.header(resip::h_Contacts).front().uri().user()+Data(" is rejected"));
		sis->reject(603);
		msgNOTIFY(Data("incoming call rejected "));
		return;
	}
	if(isStopping())
	{
		InfoLog(<<"rejecting inbound call as we are stopping");
		sis->reject(503);
		return;
	}

	// Check the headers
	if(!msg.exists(h_From))
	{
		WarningLog(<<"inbound connection missing from header, rejecting dialog");
		sis->reject(603);
		return;
	}
	// FIXME - do above for all headers
	if(msg.getReceivedTransport() == 0) 
	{
		// msg not received from the wire
		// FIXME
		WarningLog(<<"request not received from the wire");
		sis->reject(603);
	}
	Tuple sourceTuple = msg.getSource();
	Data sourceIp = Data(inet_ntoa(sourceTuple.toGenericIPAddress().v4Address.sin_addr));
	
	AppSession *call = new AppSession(this, mDum, (MyAppDialog*)sis->getAppDialog().get(), msg.header(h_From), msg.header(h_RequestLine).uri(), msg.header(h_CallId).value());

	msgIncomingCall(msg.header(resip::h_CallId).value(),
		msg.header(resip::h_Contacts).front().displayName(),
	   msg.header(resip::h_Contacts).front().uri().user(),
	   msg.header(resip::h_Contacts).front().uri().host(),
	   msg.header(resip::h_RequestLine).uri().host());

	Data frommsg("new call:");
	frommsg += msg.header(resip::h_Contacts).front().uri().toString();
	msgNOTIFY( frommsg );

	Lock lock(mMut); (void)lock;
	mCallList.push_back(call);
	call->onNewIncomingCall(sis, oat, msg);

}

void DialInstance::onNewSession(ClientInviteSessionHandle cis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
	
	MyAppDialog *myAppDialog = (MyAppDialog *)cis->getAppDialog().get();
	AppSession *call = getCallSession(cis.get());
	if(call == NULL)
	{
		WarningLog(<<"onNewSession: unrecognised dialog");
		return;
	}
	call->setBLegAppDialog(myAppDialog);

	call->onOutgoingSession(cis, oat, msg);
}

void DialInstance::onProvisional(ClientInviteSessionHandle cis, const SipMessage& msg)
{
	AppSession *call = getCallSession(cis.get());
	if(call == NULL)
	{
		WarningLog(<<"onProvisional: unrecognised dialog");
		return;
	}
	call->onProvisional(cis, msg);
	
	msgNOTIFY("Ring...");
}

void DialInstance::onEarlyMedia(ClientInviteSessionHandle cis, const SipMessage& msg, const SdpContents& sdp) 
{
	AppSession *call = getCallSession(cis.get());
	if(call == NULL)
	{
		WarningLog(<<"onEarlyMedia: unrecognised dialog");
		return;
	}
    
	Tuple sourceTuple = msg.getSource();
	unsigned long msgSourceAddress = sourceTuple.toGenericIPAddress().v4Address.sin_addr.s_addr;
	call->onEarlyMedia(cis, msg, sdp);

}

void DialInstance::onRefer(InviteSessionHandle is, ServerSubscriptionHandle ssh, const SipMessage& msg)
{
	AppSession *call = getCallSession(is.get());
	if(call == NULL)
	{
		WarningLog(<<"onConnected: unrecognised dialog");
		return;
	}
	if(call->getStatus() == AppSession::Talking)
	{
		AppSession *referSession = new AppSession(*call);
		Lock *lock = new Lock(mMut);
		mCallList.push_back(referSession);
		delete lock;

		referSession->setRemoteTransferState(AppSession::TransferTarget);
		referSession->InviteFromRefer(is, ssh, msg);		
	}

	call->setRemoteTransferState(AppSession::Transferor);
	call->onRefer(is, ssh, msg);
	
	msgNOTIFY("receive refer ");
}

void DialInstance::onReferAccepted(InviteSessionHandle is, ClientSubscriptionHandle csh, const SipMessage& msg)
{
   mResult = ReferSuccessful;
//   mProgress = Done;
//   mStream=new AudioStream();
//   mStream->ringstream_start();
}

void DialInstance::onReferRejected(InviteSessionHandle is, const SipMessage& msg)
{
   mResult = ReferUnsuccessful;
//   mProgress = Done;
}

void DialInstance::onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& isdp)
{
	MyAppDialog *myAppDialog = (MyAppDialog *)is->getAppDialog().get();
	AppSession *call = getCallSession(is.get());
	if(call == NULL)
	{
		WarningLog(<<"onOffer: unrecognised dialog");
		return;
	}
	InfoLog (<<"onOffer received");
	
//	Tuple sourceTuple = msg.getSource();
//	unsigned long msgSourceAddress = sourceTuple.toGenericIPAddress().v4Address.sin_addr.s_addr;
	call->onOffer(is, msg, isdp);  

}

void DialInstance::onOfferRequired(InviteSessionHandle sis, const SipMessage& msg)
{
	Data callid=msg.header(resip::h_CallId).value();
	AppSession* session=findAppSession(callid);
	if(!session)
	{
		InfoLog (<< "Couldn't find appsession");
		return;
	}
	session->onOfferRequired(sis, msg);
}

void DialInstance::onOfferRejected(InviteSessionHandle is, const SipMessage *msg)
{
	Data callid=msg->header(resip::h_CallId).value();
	AppSession* session=findAppSession(callid);
	if(!session)
	{
		InfoLog (<< "Couldn't find appsession");
		return;
	}
	session->onOfferRejected(is, msg);
}

void DialInstance::onAnswer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)
{
	MyAppDialog *myAppDialog = (MyAppDialog *)is->getAppDialog().get();
	AppSession *call = getCallSession(is.get());
	if(call == NULL)
	{
		WarningLog(<<"onAnswer: unrecognised dialog");
		return;
	}

	Tuple sourceTuple = msg.getSource();
	unsigned long msgSourceAddress = sourceTuple.toGenericIPAddress().v4Address.sin_addr.s_addr;
	call->onAnswer(is, msg, sdp);

}

void DialInstance::onTerminated(InviteSessionHandle is, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
{
	DebugLog(<<"onTerminated, reason = "<< reason);
	AppSession *call = getCallSession(is.get());
	if(call == NULL)
	{
		WarningLog(<<"onTerminated: unrecognised dialog");
		return;
	}
	
	call->onTerminated( is, reason, msg);
}

void 
DialInstance::addAppSession(resip::Data cid, AppSession* session)
{
	Lock lock(mMut); (void)lock;

	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getCallId()==cid)
		{
			std::list<AppSession*>::iterator tmp=iter;
			iter++;
			mCallList.erase(tmp);
			delete (*tmp);
		}
		else
		{
			iter++;
		}
	}
	mCallList.push_back(session);
}


AppSession*  
DialInstance::findAppSession(const int cid)
{
	Lock lock(mMut); (void)lock;

	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getPhoneLine())
		{
			if((*iter)->getPhoneLine()->getLineNo()==cid)
			{
				return (*iter);
			}
		}
		iter++;
	}
	return NULL;
}

AppSession* 
DialInstance::findAppSession(const resip::Data did)
{
	Lock lock(mMut); (void)lock;

	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getCallId()==did)
		{
			return (*iter);
		}
		iter++;
	}
	return NULL;
}

std::vector<AppSession*> 
DialInstance::findAppSessionList(const int cid)
{
	Lock lock(mMut); (void)lock;

	std::vector<AppSession*> rec;

	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getPhoneLine())
		{
			if((*iter)->getPhoneLine()->getLineNo()==cid)
			{
				rec.push_back(*iter);
			}
		}
		iter++;
	}
	return rec;
}

void 
DialInstance::eraseAppSession(resip::Data cid )
{
	Lock lock(mMut); (void)lock;

	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getCallId()==cid)
		{
			std::list<AppSession*>::iterator tmp=iter;
			iter++;
			mCallList.erase(tmp);
			return;
		}
		iter++;
	}
}

void DialInstance::eraseAppSession(AppSession* is)
{
	Lock lock(mMut); (void)lock;

	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)==is)
		{
			std::list<AppSession*>::iterator tmp=iter;
			iter++;
			delete (*tmp);
			mCallList.erase(tmp);

			return;
		}
		iter++;
	}
}

void DialInstance::addLine(int id, PhoneLine* line)
{
	std::map<int,PhoneLine*>::iterator iter = mLineMap.find(id);
	if(iter!=mLineMap.end())
	{
		delete iter->second;
		mLineMap.erase(iter);
	}
	mLineMap[id]=line;
}
PhoneLine* DialInstance::findLine(int id)
{
	std::map<int,PhoneLine*>::iterator iter = mLineMap.find(id);
	if(iter!=mLineMap.end())
	{
		return iter->second;
	}
	return NULL;
}

void DialInstance::eraseLine(int id)
{
	std::map<int,PhoneLine*>::iterator iter = mLineMap.find(id);
	if(iter!=mLineMap.end())
	{
		delete iter->second;
		mLineMap.erase(iter);
	}
	else
	{
		InfoLog (<< "Couldn't find "<<id<<" to remove");
		assert(0);
	}
}

bool DialInstance::LineIsOpen(int nLineNo)
{
	PhoneLine* line=findLine(nLineNo);
	return (line!=NULL);
}

bool DialInstance::LineIsHold(int nLineNo)
{
	PhoneLine* line=findLine(nLineNo);
	if(line)
	{
		return line->getHoldState();
	}
	return false;
}

bool DialInstance::LineIsBusy(int nLineNo)
{
	PhoneLine* line=findLine(nLineNo);
	if(line)
	{
		return line->getLineState();
	}
	return false;
}

bool DialInstance::DigitDTMF(int nLineNo, Data sDigit)
{
	AppSession* session=findAppSession(nLineNo);
	if(session)
	{
		if(session->getStatus()==AppSession::Talking)
		{
			session->mStream->SendDTMF(sDigit);
		}
	}
	mscard->SetDTMFValue(sDigit);
	return true;
}

bool DialInstance::SetDTMFVolume(int nVolume)
{
	return true;
}

int DialInstance::GetDTMFVolume()
{
	return 0;
}

bool DialInstance::EnableInbandDTMF(int nLineNo,bool dtmf)
{
	return true;
}

int DialInstance::GetMicSoundLevel()
{
	int level=0;

	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getStatus()==AppSession::Talking)
		{
			level+=(*iter)->mStream->GetMicSoundLevel();
		}
		iter++;
	}
	return level;
}

int DialInstance::GetSpkSoundLevel()
{
	int level=0;

	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getStatus()==AppSession::Talking)
		{
			level+=(*iter)->mStream->GetSpkSoundLevel();
		}
		iter++;
	}
	return level;
}


bool DialInstance::MuteMic(bool bMute)
{
	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getPhoneLine())
		{
			if((*iter)->getPhoneLine()->getLineState()==true)
			{
				(*iter)->mStream->SetMicMute(bMute);
			}
		}
		iter++;
	}
	mscard->SetMicMute(bMute);
	return true;
}

bool DialInstance::MuteSpk(bool bMute)
{
	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getPhoneLine())
		{
			if((*iter)->getPhoneLine()->getLineState()==true)
			{
				(*iter)->mStream->SetSpkMute(bMute);
			}
		}
		iter++;
	}
	mscard->SetSpkMute(bMute);
	return true;
}
	
int DialInstance::GetMicVolume()
{
	/*
	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getPhoneLine())
		{
			if((*iter)->getPhoneLine()->getLineState()==true)
			{
				return (*iter)->mStream->GetMicVolume();
			}
		}
		iter++;
	}*/
	return mscard->GetMicLevel();
}

bool DialInstance::SetMicVolume(int nVolume)
{
	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getPhoneLine())
		{
			if((*iter)->getPhoneLine()->getLineState()==true)
			{
				(*iter)->mStream->SetMicVolume(nVolume);
			}
		}
		iter++;
	}
	mscard->SetMicLevel(nVolume);
	return true;
}

int DialInstance::GetSpkVolume()
{
	/*
	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getPhoneLine())
		{
			if((*iter)->getPhoneLine()->getLineState()==true)
			{
				return (*iter)->mStream->GetSpkVolume();
			}
		}
		iter++;
	}*/
	return mscard->GetSpkLevel();
}

bool DialInstance::SetSpkVolume(int nVolume)
{
	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getPhoneLine())
		{
			if((*iter)->getPhoneLine()->getLineState()==true)
			{
				(*iter)->mStream->SetSpkVolume(nVolume);
			}
		}
		iter++;
	}
	mscard->SetSpkLevel(nVolume);
	return true;
}

void DialInstance::setOutAudioDevice(int val)
{
	mscard->SetDTMFDevice( val );
}

void DialInstance::setEchoNoiseCancellation(bool ec)
{
	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getStatus()==AppSession::Talking)
		{
			(*iter)->mStream->SetEchoCancel(ec);
		}
		iter++;
	}
}

void DialInstance::setAGCLevel(int level)
{
	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getStatus()==AppSession::Talking)
		{
			(*iter)->mStream->SetAGC(level);
		}
		iter++;
	}
}

void DialInstance::setMicBoost(bool boost)
{
	std::list<AppSession*>::iterator iter = mCallList.begin();
	while(iter!=mCallList.end())
	{
		if((*iter)->getStatus()==AppSession::Talking)
		{
			(*iter)->mStream->SetMicBoost(boost);
		}
		iter++;
	}
}

bool DialInstance::IsRecording(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->IsRecording();
	}
	return false;
}

bool DialInstance::StartRecording(int nLineNo, int nRecordVoice, bool bRecordCompress)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->StartRecording(nRecordVoice, bRecordCompress);
	}
	return false;
}

bool DialInstance::StopRecording(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->StopRecording();
	}
	return false;
}

bool DialInstance::ResetRecording(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->ResetRecording();
	}
	return false;
}

bool DialInstance::SaveRecordingToWaveFile(int nLineNo, Data sFileName)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->SaveRecordingToWaveFile(sFileName);
	}
	return false;
}

bool DialInstance::IsWaveFilePlaying(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->IsWaveFilePlaying();
	}
	return false;
}

bool DialInstance::PlayWaveOpen(int nLineNo, Data sFileName)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->PlayWaveOpen(sFileName);
	}
	return false;
}

bool DialInstance::PlayWaveSkipTo(int nLineNo, int nSeconds)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->PlayWaveSkipTo(nSeconds);
	}
	return false;
}

int DialInstance::PlayWaveTotalTime(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return -1;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->PlayWaveTotalTime();
	}
	return -1;
}

bool DialInstance::PlayWavePause(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->PlayWavePause();
	}
	return false;
}

bool DialInstance::PlayWaveStart(int nLineNo, bool bListen)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->PlayWaveStart(bListen);
	}
	return false;
}

bool DialInstance::PlayWaveStop(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->PlayWaveStop();
	}
	return false;
}

int DialInstance::PlayWavePosition(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		return false;
	}
	AudioStream *pStream = session->mStream;
	if(pStream)
	{
		return pStream->PlayWavePosition();
	}
	return false;
}

bool DialInstance::SetTOS(int nLineNo, int nValue)
{
	return true;
}

int DialInstance::GetTOS(int nLineNo)
{
	return 0;
}

int DialInstance::GetOutboundCodec(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		assert(0);
		return -1;
	}
	return session->GetOutboundCodec();
}

int DialInstance::GetInboundCodec(int nLineNo)
{
	AppSession* session=findAppSession(nLineNo);
	if(!session)
	{
		msgNOTIFY(Data("line doesn't exist"));
		assert(0);
		return -1;
	}
	return session->GetInboundCodec();
}

// Get rid of punctuation like `.' and `-'
// Keep a leading `+' if present
// assert if not a real number
Data DialInstance::processNumber(const Data& verboseNumber)
{
   Data num = Data("");
   int len = verboseNumber.size();
   for(int i = 0; i < len; i++)
   {
      char c = verboseNumber[i];
      switch(c)
      {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         num.append(&c, 1);
         break;
      case '+':
         assert(i == 0);   // FIXME - better error handling needed
         num.append(&c, 1);
         break;
      case '.':
      case '-':
         // just ignore those characters
         break;
      default:
         // any other character is garbage
         assert(0);
      }
   }
   return num;
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

