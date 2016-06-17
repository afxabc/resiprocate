
#include <exception>
#include "AppSession.hxx"
#include "DialInstance.hxx"
#include "MyAppDialogSet.hxx"
#include "rutil/stun/Stun.hxx"
#include "MyAudioStream.hxx"
#ifdef VIDEO_ENABLE
#include "MyVideoStream.hxx"
#endif
#include "rutil/Logger.hxx"
#include "PhoneLine.hxx"


#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace resip;
using namespace std;

Data AppSession::callStateNames[] = {
     Data("Undefined"),
     Data("Connected"),
     Data("SentUpdate"),
     Data("SentUpdateGlare"),
     Data("SentReinvite"),
     Data("SentReinviteGlare"),
     Data("SentReinviteNoOffer"),
     Data("SentReinviteAnswered"),
     Data("SentReinviteNoOfferGlare"),
     Data("ReceivedUpdate"),
     Data("ReceivedReinvite"),
     Data("ReceivedReinviteNoOffer"),
     Data("ReceivedReinviteSentOffer"),
     Data("Answered"),
     Data("WaitingToOffer"),
     Data("WaitingToRequestOffer"),
     Data("WaitingToTerminate"),
     Data("WaitingToHangup"),
     Data("Terminated"),
     Data("TermKill"),

     Data("CallerStart"),
     Data("CallerEarly"),
     Data("CallerEarlyWithOffer"),
     Data("CallerEarlyWithAnswer"),
     Data("CallerAnswered"),
     Data("CallerSentUpdateEarly"),
     Data("CallerSentUpdateEarlyGlare"),
     Data("CallerReceivedUpdateEarly"),
     Data("CallerSentAnswer"),
     Data("CallerQueuedUpdate"),
     Data("CallerCancelled"),

     Data("CalleeStart"),
     Data("CalleeOffer"), 
     Data("CalleeOfferProvidedAnswer"),
     Data("CalleeEarlyOffer"),
     Data("CalleeEarlyProvidedAnswer"), 

     Data("CalleeNoOffer"), 
     Data("CalleeProvidedOffer"), 
     Data("CalleeEarlyNoOffer"), 
     Data("CalleeEarlyProvidedOffer"), 
     Data("CalleeAccepted"), 
     Data("CalleeWaitingToOffer"), 
     Data("CalleeWaitingToRequestOffer"), 

     Data("CalleeAcceptedWaitingAnswer"), 
     Data("CalleeReceivedOfferReliable"),
     Data("CalleeNoOfferReliable"),
     Data("CalleeFirstSentOfferReliable"),
     Data("CalleeFirstSentAnswerReliable"),
     Data("CalleeNegotiatedReliable"),
     Data("CalleeSentUpdate"),
     Data("CalleeSentUpdateAccepted"),
     Data("CalleeReceivedUpdate"),
     Data("CalleeReceivedUpdateWaitingAnswer"),
     Data("CalleeWaitingToTerminate"),
     Data("CalleeWaitingToHangup")
};

unsigned int AppSession::T0 = 5;

const char *AppSession::basicClearingReasonName[] = {
  "NO ANSWER",
  "BUSY",
  "CONGESTION",
  "ERROR",
  "ANSWERED"
};

/*for callee*/
AppSession::AppSession(DialInstance *dial,
		DialogUsageManager* dum, 
		MyAppDialog *aAppDialog,
		const resip::NameAddr& sourceAddr, 
		const resip::Uri& destinationAddr,
		const resip::Data callId):mSourceAddr(sourceAddr),
							mDestinationAddr(destinationAddr),
							mLine(NULL),
							mCallId(callId),
							mInputDevice(0),
							mOutputDevice(0),
							mRTPRxPort(17078),
							mFClearingReason(Unset),
							mRejectOtherCode(0),
							mRemotePoint(NoTransfer),
							mFailureStatusCode(-1),
							bLegAppDialog(NULL),
							bLegAppDialogSet(NULL),
							aLegAppDialog(aAppDialog),
							mCallState(Undefined),
							mConnectTime(0),
							mFinishTime(0),
							mFailureReason(NULL),
							mParent(dial),
							mDum(dum)
#ifdef VIDEO_ENABLE
							,
							mVideoRxPort(19078)
#endif
{

	aLegAppDialog->setCallSession(this);

	time(&mStartTime);

	mStream = new MyAudioStream(this);
	mStream->SetConference(mParent->getAudioConf());

#ifdef VIDEO_ENABLE
	mVideoStream = new MyVideoStream(this);
#endif

	mStream->miclevel=mParent->getMSSndCard()->GetMicLevel();
	mStream->micmute=mParent->getMSSndCard()->GetMicMute();
	mStream->spklevel=mParent->getMSSndCard()->GetSpkLevel();
	mStream->spkmute=mParent->getMSSndCard()->GetSpkMute();

	mInputDevice = mParent->getDialerConfig().getInAudioDev();
	mOutputDevice = mParent->getDialerConfig().getOutAudioDev();
}

/*for caller*/
AppSession::AppSession(DialInstance *dial,
		DialogUsageManager* dum, 
		const resip::NameAddr& sourceAddr, 
		const resip::Uri& destinationAddr,
		PhoneLine* line,
		int inputDevice,
		int outputDevice):	mSourceAddr(sourceAddr),
					mDestinationAddr(destinationAddr), 
					mInputDevice(inputDevice),
					mOutputDevice(outputDevice),
					mFClearingReason(Unset),
					mRejectOtherCode(0),
					mRemotePoint(NoTransfer),
					mFailureStatusCode(-1),
					aLegAppDialog(NULL),
					bLegAppDialog(NULL),
					bLegAppDialogSet(NULL),
					mCallState(Undefined),
					mConnectTime(0),
					mFinishTime(0),
					mLine(line),
					mParent(dial),
					mDum(dum)				
{
	time(&mStartTime);
	mLine->AddSession(this);
	mLine->setLineState(true);
	
	mStream = new MyAudioStream(this);
	mStream->SetConference(mParent->getAudioConf());

#ifdef VIDEO_ENABLE
	mVideoStream = new MyVideoStream(this);
#endif

	mStream->miclevel=mParent->getMSSndCard()->GetMicLevel();
	mStream->micmute=mParent->getMSSndCard()->GetMicMute();
	mStream->spklevel=mParent->getMSSndCard()->GetSpkLevel();
	mStream->spkmute=mParent->getMSSndCard()->GetSpkMute();
	mFailureReason = NULL;
	mRTPRxPort=mLine->getRTPRxPort();
#ifdef VIDEO_ENABLE
	mVideoRxPort=mLine->getRTPRxVideoPort();
#endif
}

AppSession::AppSession(const AppSession& session):
	mInputDevice(session.mInputDevice),
		mOutputDevice(session.mOutputDevice),
		mFClearingReason(session.mFClearingReason),
		mRejectOtherCode(0),
		mRemotePoint(NoTransfer),
		mFailureStatusCode(-1),
		aLegAppDialog(NULL),
		bLegAppDialog(NULL),
		bLegAppDialogSet(NULL),
		mCallState(Undefined),
		mConnectTime(0),
		mFinishTime(0),
		mLine(session.mLine),
		mParent(session.mParent),
		mDum(session.mDum)				
{
	time(&mStartTime);
	mLine->AddSession(this);
	mStream = new MyAudioStream(this);
	mStream->SetConference(mParent->getAudioConf());

#ifdef VIDEO_ENABLE
	mVideoStream = new MyVideoStream(this);
#endif

	mStream->miclevel=mParent->getMSSndCard()->GetMicLevel();
	mStream->micmute=mParent->getMSSndCard()->GetMicMute();
	mStream->spklevel=mParent->getMSSndCard()->GetSpkLevel();
	mStream->spkmute=mParent->getMSSndCard()->GetSpkMute();
	mFailureReason = NULL;
	mRTPRxPort=mLine->getRTPRxPort()+10;
#ifdef VIDEO_ENABLE
	mVideoRxPort=mLine->getRTPRxVideoPort()+10;
#endif
}

AppSession::~AppSession()
{
	InfoLog (<< "delete appsession");

	if(mLine)
	{
		mLine->RemSession(this);
	}
	if(mStream != NULL)
		delete mStream;
		
#ifdef VIDEO_ENABLE
	if(mVideoStream != NULL)
		delete mVideoStream;
#endif

	if(mFailureReason != NULL)
		delete mFailureReason;
	
	if(aLegAppDialog != NULL)
	{
		InfoLog (<< "aLegAppDialog->setCallSession=NULL");
		aLegAppDialog->setCallSession(NULL);
	}
	
	if(bLegAppDialogSet != NULL)
	{
		InfoLog (<< "bLegAppDialogSet->setCallSession=NULL)");
		bLegAppDialogSet->setCallSession(NULL);
	}
	
	if(bLegAppDialog != NULL)
	{
		InfoLog (<< "bLegAppDialog->setCallSession=NULL");
		bLegAppDialog->setCallSession(NULL);
	}
}

bool AppSession::setCallState(CallState newCallState)
{
	DebugLog(<<"CallState change: " << getCallStateName(mCallState) << " -> " << getCallStateName(newCallState));

	mCallState = newCallState;
	return true;
}

const resip::Data& AppSession::getCallStateName(CallState s)
{
	return callStateNames[s];
}

void AppSession::setBLegAppDialog(MyAppDialog *myAppDialog)
{
  bLegAppDialog = myAppDialog; 
}

void AppSession::releaseAppDialog(MyAppDialog *myAppDialog)
{
  if(myAppDialog == aLegAppDialog)
  {
    aLegAppDialog = NULL;
  }
  if(myAppDialog == bLegAppDialog)
  {
    bLegAppDialog = NULL;
  }

}

void AppSession::releaseAppDialogSet(MyAppDialogSet *myAppDialogSet)
{
  if(myAppDialogSet == bLegAppDialogSet)
  {
    bLegAppDialogSet = NULL;
  }
  else
  {
    ErrLog(<<"releaseAppDialogSet for unknown AppDialogSet");
  }
}

/* This method is called regularly.
  Instead, the callbacks or setCallState should send a message to 
  B2BUA to say that the call needs attention. */
void AppSession::checkProgress(time_t now, bool stopping)
{
	checkMessage();

  switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
		break;
	case Terminated:
		setCallState(TermKill);
		break;
	case CallerStart:
	case CallerEarly:
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:
		break;

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:
	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::checkMessage()
{
	if( mMessageList.size() > 0 )
	{
		AppMessage *msg = mMessageList.front();
		mMessageList.pop_front();
		switch(msg->Type)
		{
		case AppMessage::OutCallMsg:
			doOutgoingCall();
			break;
		case AppMessage::SendAnswerMsg:
			CallAccept(msg->Indev,msg->Outdev);
			break;
		case AppMessage::SendRejectMsg:
			CallReject();
			break;
		case AppMessage::SendHangupMsg:
			Disconnect();
			break;
		case AppMessage::HoldLineMsg:
			LineHold();
			break;
		case AppMessage::UnholdLineMsg:
			LineUnHold();
			break;
		case AppMessage::TransferCallMsg:
			CallTransfer(msg->ToUri);
			break;
		default:
			assert(0);
			break;
		}
		delete msg;
	}
}

bool AppSession::isComplete() 
{
  return (mCallState == TermKill);
}

AppSession::CallStatus AppSession::getStatus()
{
	CallStatus state = Unknown;

  switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
		state = Talking;
		break;
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
		break;
	case CallerEarly:
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
		break;
	case CalleeEarlyOffer:
		break;
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
		break;
	case CalleeEarlyNoOffer:
		break;
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
  return state;
}

void AppSession::doOutgoingCall()
{
	setCallState(CallerStart);

	try {
		//SharedPtr<UserProfile> outboundUserProfile(new UserProfile);
		SharedPtr<UserProfile> outboundUserProfile(mDum->getMasterUserProfile());
		outboundUserProfile->setDefaultFrom(NameAddr(mParent->getDialerConfig().getDialerIdentity()));
		
		bLegAppDialogSet = new MyAppDialogSet(*mDum, this, outboundUserProfile);
		initialCommon();
		buildSdpTemplate((SdpContents*)&mOutsdp);

		int rtpport;
		if( mAudioParams.getNatPort() > 0 )
		{
			rtpport =mAudioParams.getNatPort();
		}
		else if( mParent->getDialerConfig().getOutboundProxy() != Data::Empty )
		{
			rtpport = mParent->getDialerConfig().getOutboundPort().convertInt();
		}
		else
		{
			rtpport = mRTPRxPort;
		}

		SdpContents::Session::Medium media(Symbols::audio,rtpport,1,Symbols::RTP_AVP);
		buildCodecs((SdpContents::Session::Medium*)&media);
		SdpContents::Session::Codec codec0("telephone-event", 101, 8000);
		media.addCodec(codec0);
		media.addAttribute("fmtp", "101 0-15");
		media.addAttribute("sendrecv");

		mOutsdp.session().addMedium(media);

#ifdef VIDEO_ENABLE
		if(mParent->getDialerConfig().getVideoCapture())
		{
			int videortpport=mVideoParams.getNatPort()>0?mVideoParams.getNatPort():mVideoRxPort;

			SdpContents::Session::Medium vediomedia(Symbols::video,videortpport,1,Symbols::RTP_AVP);
			buildVideoCodecs((SdpContents::Session::Medium*)&vediomedia);
			vediomedia.addAttribute("sendrecv");

			mOutsdp.session().addMedium(vediomedia);
		}
#endif

		SharedPtr<SipMessage> msg = mDum->makeInviteSession(NameAddr(mDestinationAddr), outboundUserProfile, (SdpContents*)&mOutsdp, bLegAppDialogSet);

		mDum->send(msg);

		mParent->msgNOTIFY("contacting distination host");

	} 
	catch (...) 
	{
		WarningLog(<<"failed to create new InviteSession");
//		setClearingReason(AuthError, -1);
		setCallState(Terminated);
		return;
	} 
}

bool AppSession::Disconnect()
{
	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
		{
			if(mLine)
			{
				mParent->msgFailToConnect(mLine->getLineNo());
				mLine->setLineState(false);
				mLine->setHoldState(false);
			}
			InfoLog (<< "call terminated");
			mParent->msgNOTIFY("call terminated ");
			setCallState(Terminated);
		}
		break;
	case CallerEarly:
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
		setCallState(CallerCancelled);
		break;
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
		break;
	case CalleeEarlyOffer:
		break;
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
		break;
	case CalleeEarlyNoOffer:
		break;
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}

	DisconnectCall();
	return true;
}

bool AppSession::CallReject()
{
	ServerInviteSession *sis = (ServerInviteSession *)(aLegAppDialog->getInviteSession().get());
		
	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
	case CallerStart:
	case CallerEarly:
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:
		break;
	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:
	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:
	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
		sis->reject(603);
		break;
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}

	return true;
}

void AppSession::DisconnectCall()
{
	time(&mWaitDisTime);

	if(aLegAppDialog != NULL) 
	{
		ServerInviteSession *sis = (ServerInviteSession *)(aLegAppDialog->getInviteSession().get());
		sis->end();
	} 
	if(bLegAppDialogSet != NULL)
	{
		bLegAppDialogSet->end();
	}
	
}

void AppSession::onFailure(ClientInviteSessionHandle cis, const SipMessage& msg)
{
}

void AppSession::onTerminated(InviteSessionHandle is, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
{
	
	switch(reason) 
	{
	case InviteSessionHandler::Error:
		if(mLine)
		{
			if(this->mRemotePoint == AppSession::TransferTarget)
			{
				mParent->msgTransferFail(mLine->getLineNo());
			}
			else
			{
				mParent->msgFailToConnect(mLine->getLineNo());
			}
		}
		break;
	case InviteSessionHandler::Timeout: 
		break;
	case InviteSessionHandler::Replaced:
		break;
	case InviteSessionHandler::LocalBye:
		if(mLine)
			mParent->msgDisconnectCall(mLine->getLineNo());
		break;
	case InviteSessionHandler::RemoteBye:
		if(this->mRemotePoint == AppSession::Transferor)
		{
			mParent->msgTransferSuccess(mLine->getLineNo());
			break;
		}
		if(mCallState <= Terminated)
		{
			if(mLine)
				mParent->msgDisconnectCall(mLine->getLineNo());
		}
		else if(mCallState <= CallerCancelled)
		{
			if(mLine)
				mParent->msgFailToConnect(mLine->getLineNo());
		}
		else
		{
			mParent->msgIncomingCallMissed(mCallId);
		}
		break;
	case InviteSessionHandler::LocalCancel:
		if(mLine)
			mParent->msgDisconnectCall(mLine->getLineNo());
		break;
	case InviteSessionHandler::RemoteCancel:
		break;
	case InviteSessionHandler::Rejected: //Only as UAS, UAC has distinct onFailure callback
		mParent->msgIncomingCallMissed(mCallId);
		break;
	case InviteSessionHandler::Referred:
		break;
	default:
		break;
	}

	time(&mFinishTime);

	if(mStream->isRunning())
	{
		mStream->audiostream_stop();
	}

#ifdef VIDEO_ENABLE
   if(mVideoStream!=NULL)
   {
	   if(mVideoStream->isRunning())
		   mVideoStream->videostream_stop();
   }
#endif

	if(mLine)
	{
		mLine->setLineState(false);
		mLine->setHoldState(false);
	}
	InfoLog (<< "call terminated");
	mParent->msgNOTIFY("call terminated ");

	setCallState(Terminated);

}

void AppSession::onRefer(InviteSessionHandle is, ServerSubscriptionHandle ssh, const SipMessage& msg)
{
	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
		ReciveTransfer(is, ssh, msg);
		break;
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
	case CallerStart:
	case CallerEarly:
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:
		break;
	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:
	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:
	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
		break;
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::ReciveTransfer(InviteSessionHandle is, ServerSubscriptionHandle ssh, const SipMessage& msg)
{
	SharedPtr<SipMessage> ans = ssh->accept();
	ssh->send(ans);

	if(mLine)
	{
		Data addr = msg.header(resip::h_ReferTo).uri().user();// + Data("@") + msg.header(resip::h_ReferTo).uri().host();
		mParent->msgTransferStart(mLine->getLineNo(), addr);
	}
}

void AppSession::InviteFromRefer(InviteSessionHandle is, ServerSubscriptionHandle ssh, const SipMessage& msg)
{
	setCallState(CallerStart);

	SharedPtr<UserProfile> outboundUserProfile(mDum->getMasterUserProfile());
	outboundUserProfile->setDefaultFrom(NameAddr(mParent->getDialerConfig().getDialerIdentity()));
	
	bLegAppDialogSet = new MyAppDialogSet(*mDum, this, outboundUserProfile);
	initialCommon();
	buildSdpTemplate((SdpContents*)&mOutsdp);
	
	int rtpport;
	if( mAudioParams.getNatPort() > 0 )
	{
		rtpport =mAudioParams.getNatPort();
	}
	else if( mParent->getDialerConfig().getOutboundProxy() != Data::Empty )
	{
		rtpport = mParent->getDialerConfig().getOutboundPort().convertInt();
	}
	else
	{
		rtpport = mRTPRxPort;

	}

	SdpContents::Session::Medium media(Symbols::audio,rtpport,1,Symbols::RTP_AVP);
	buildCodecs((SdpContents::Session::Medium*)&media);
	SdpContents::Session::Codec codec0("telephone-event", 101, 8000);
	media.addCodec(codec0);
	media.addAttribute("fmtp", "101 0-15");
	media.addAttribute("sendrecv");
	mOutsdp.session().addMedium(media);
			
#ifdef VIDEO_ENABLE
	if(mParent->getDialerConfig().getVideoCapture())
	{
		int videortpport=mVideoParams.getNatPort()>0?mVideoParams.getNatPort():mVideoRxPort;
		SdpContents::Session::Medium vediomedia(Symbols::video,videortpport,1,Symbols::RTP_AVP);
		
		buildVideoCodecs((SdpContents::Session::Medium*)&vediomedia);
		vediomedia.addAttribute("sendrecv");
		
		mOutsdp.session().addMedium(vediomedia);
	}
#endif

	SharedPtr<SipMessage> transferInvite = mDum->makeInviteSessionFromRefer(msg, ssh, &mOutsdp, bLegAppDialogSet);
	ssh->send(transferInvite);
}

void AppSession::onStopping()
{
  // forcible to stop all appsessions.
	DisconnectCall();
}

bool AppSession::CallAccept( int nInputDeviceId, int nOutputDeviceId)
{	
//	mLine=nLine;
	mLine->AddSession(this);
	mLine->setLineState(true);
	mRTPRxPort = mLine->getRTPRxPort();
	mInputDevice=nInputDeviceId;
	mOutputDevice=nOutputDeviceId;

	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
		break;
	case CallerEarly:
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
		break;
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:
		setCallState(CalleeEarlyProvidedAnswer);
		AcceptOfferCall();
		break;

	case CalleeNoOffer:
	case CalleeProvidedOffer:
		break;
	case CalleeEarlyNoOffer:
		setCallState(CalleeEarlyProvidedOffer);
		AcceptNoOfferCall();
		break;
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}

	return true;
}

void AppSession::AcceptOfferCall()
{
	initialCommon();

	buildSdpTemplate((SdpContents*)&mOutsdp);

	int rtpport;
	if( mAudioParams.getNatPort() > 0 )
	{
		rtpport =mAudioParams.getNatPort();
	}
	else if( mParent->getDialerConfig().getOutboundProxy() != Data::Empty )
	{
		rtpport = mParent->getDialerConfig().getOutboundPort().convertInt();
	}
	else
	{
		rtpport = mRTPRxPort;
	}
	
	SdpContents::Session::Medium media(Symbols::audio,rtpport,1,Symbols::RTP_AVP);
	buildCodecs((SdpContents::Session::Medium*)&media);
	SdpContents::Session::Codec codec0("telephone-event", 101, 8000);
    media.addCodec(codec0);
	media.addAttribute("fmtp", "101 0-15");
//	media.addAttribute("sendrecv");

	ServerInviteSession *sis = (ServerInviteSession *)(aLegAppDialog->getInviteSession().get());
	
	SdpContents::Session::Medium tmpMedia(media);
	media.clearCodecs();
	std::list<SdpContents::Session::Codec>::const_iterator sIter;
	for (sIter=tmpMedia.codecs().begin(); sIter!=tmpMedia.codecs().end(); ++sIter)
	{
		if(sIter->payloadType()==mAudioParams.getPayType())
		{
			break;
		}
	}
	if(sIter!=tmpMedia.codecs().end())
	{
		media.addCodec(*sIter);
		if((*sIter).getName()==Data("iLBC"))
		{
			media.addAttribute("fmtp", "97 mode=30");
		}
	}
	media.addCodec(codec0);
	media.addAttribute("fmtp", "101 0-15");
	media.addAttribute("sendrecv");
	mOutsdp.session().addMedium(media);

#ifdef VIDEO_ENABLE
	if(mParent->getDialerConfig().getVideoCapture())
	{
		int videortpport=mVideoParams.getNatPort()>0?mVideoParams.getNatPort():mVideoRxPort;

		SdpContents::Session::Medium vediomedia(Symbols::video,videortpport,1,Symbols::RTP_AVP);
		buildVideoCodecs((SdpContents::Session::Medium*)&vediomedia);
		vediomedia.addAttribute("sendrecv");

		mOutsdp.session().addMedium(vediomedia);
	}
#endif
		
	setCallState(CalleeEarlyProvidedAnswer);

	sis->provideAnswer(mOutsdp);

	setCallState(CalleeAccepted);

	sis->accept();

}

void AppSession::AcceptNoOfferCall()
{
	initialCommon();

	buildSdpTemplate((SdpContents*)&mOutsdp);

	int rtpport;
	if( mAudioParams.getNatPort() > 0 )
	{
		rtpport =mAudioParams.getNatPort();
	}
	else if( mParent->getDialerConfig().getOutboundProxy() != Data::Empty )
	{
		rtpport = mParent->getDialerConfig().getOutboundPort().convertInt();
	}
	else
	{
		rtpport = mRTPRxPort;
	}
	
	SdpContents::Session::Medium media(Symbols::audio,rtpport,1,Symbols::RTP_AVP);
	buildCodecs((SdpContents::Session::Medium*)&media);
	SdpContents::Session::Codec codec0("telephone-event", 101, 8000);
    media.addCodec(codec0);
	media.addAttribute("fmtp", "101 0-15");
	media.addAttribute("sendrecv");

	ServerInviteSession *sis = (ServerInviteSession *)(aLegAppDialog->getInviteSession().get());
	
	mOutsdp.session().addMedium(media);

#ifdef VIDEO_ENABLE
	if(mParent->getDialerConfig().getVideoCapture())
	{
		int videortpport=mVideoParams.getNatPort()>0?mVideoParams.getNatPort():mVideoRxPort;

		SdpContents::Session::Medium vediomedia(Symbols::video,videortpport,1,Symbols::RTP_AVP);
		buildVideoCodecs((SdpContents::Session::Medium*)&vediomedia);
		vediomedia.addAttribute("sendrecv");

		mOutsdp.session().addMedium(vediomedia);
	}
#endif

	sis->provideOffer(mOutsdp);

	setCallState(CalleeAcceptedWaitingAnswer);
	sis->accept();

}

bool AppSession::CallTransfer(resip::Data sToURI)
{
	InviteSession *is;
	if (aLegAppDialog)
	{
		is = (InviteSession *)(aLegAppDialog->getInviteSession().get());
	}
	else if (bLegAppDialog)
	{
		is = (InviteSession *)(bLegAppDialog->getInviteSession().get());
	}
	else
	{
		return false;
	}

	is->refer(NameAddr(sToURI));

/*	if(mLine)
	{
		mParent->msgTransferStart(mLine->getLineNo(), sToURI);
	}*/
	return true;
}

bool AppSession::LineHold()
{
	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
		setCallState(SentReinvite);
		HoldtheLine();
		break;
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
	case CallerStart:
	case CallerEarly:
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
	return true;
}

void AppSession::HoldtheLine()
{
	if( mLine->getHoldState() ) return;
	
	SdpContents::Session::Medium media = mOutsdp.session().media().front();
	mOutsdp.session().media().pop_front();
	media.clearAttribute("sendrecv");
	media.addAttribute("sendonly");
	mOutsdp.session().addMedium(media);

	InviteSession *is;
	if(aLegAppDialog) 
	{
		is = (InviteSession *)(aLegAppDialog->getInviteSession().get());
	}
	else if(bLegAppDialog)
	{
		 is = (InviteSession *)(bLegAppDialog->getInviteSession().get());
	}
	is->provideOffer(mOutsdp);

}

bool AppSession::LineUnHold()
{
	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
		setCallState(SentReinvite);
		UnHoldtheLine();
		break;
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
		break;
	case ReceivedReinvite:
		break;
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
	case CallerStart:
	case CallerEarly:
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
	return true;
}

void AppSession::UnHoldtheLine()
{
	if( !(mLine->getHoldState()) ) return;

	SdpContents::Session::Medium media=mOutsdp.session().media().front();
	mOutsdp.session().media().pop_front();
	media.clearAttribute("sendonly");
	media.addAttribute("sendrecv");
	mOutsdp.session().addMedium(media);

	InviteSession *is;
	if(aLegAppDialog != NULL) 
	{
		is = (InviteSession *)(aLegAppDialog->getInviteSession().get());
	}
	else if(bLegAppDialog)
	{
		 is = (InviteSession *)(bLegAppDialog->getInviteSession().get());
	}
	is->provideOffer(mOutsdp);
	
}


void AppSession::onOutgoingSession( ClientInviteSessionHandle cis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
}

void AppSession::onNewIncomingCall(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
	switch(this->mCallState)
	{
	case Undefined:
		setCallState(CalleeStart);
		RecivedNewInCall(sis, oat, msg);
		break;
	case Connected:
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
		break;
	case CallerEarly:
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::RecivedNewInCall(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
	if(!aLegAppDialog)
	{
		return;
	}
	sis->provisional(100);
	
	mParent->msgIncomingCallRingingStart(mCallId);
}

void AppSession::onOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)
{
	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
		setCallState(ReceivedReinvite);
		RemoteHold(is, msg, sdp);
		break;
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
		break;
	case CallerEarly:
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:
		break;

	case CalleeStart:
		setCallState(CalleeOffer);
		ReceivedOffer(is, msg, sdp);
		break;
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::ReceivedOffer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)
{
//	if(mCallState == CallActive) return;/*call hold*/
	Data ipaddr=sdp.session().connection().getAddress();
	if(ipaddr==Data::Empty)
	{
		ipaddr=sdp.session().origin().getAddress();
	}
	mAudioParams.setRemoteAddr(ipaddr);

	for (list<SdpContents::Session::Medium>::const_iterator i = sdp.session().media().begin(); 
		i != sdp.session().media().end(); ++i)
	{
		if(i->name() == Symbols::audio)
		{
			int rtpport=i->port();
			mAudioParams.setRemotePort(rtpport);

			SdpContents::Session::Medium medium;
			buildCodecs((SdpContents::Session::Medium*)&medium);

			SdpContents::Session::Codec incode=medium.findFirstMatchingCodecs(*i);
			if((incode.getName()==Data::Empty)||(incode.payloadType()==TELEPHONE_EVENT))
			{
				mParent->msgNOTIFY("media not supported ");
				DisconnectCall();
				//mParent->msgIncomingCallRingingStop(mCallId);
				return;
			}
			int pt=incode.payloadType();
			mInboundCodec=pt;
			mOutboundCodec=pt;
			mAudioParams.setPayType(pt);
		}
#ifdef VIDEO_ENABLE
		else if(i->name() == Symbols::video)
		{
			if(mParent->getDialerConfig().getVideoPlay())
			{
				SdpContents::Session::Medium medium;
				buildVideoCodecs((SdpContents::Session::Medium*)&medium);

				SdpContents::Session::Codec incode=medium.findFirstMatchingCodecs(*i);
				if(incode.getName()==Data::Empty)
				{
					mParent->msgNOTIFY("video media not supported ");
				}
				else
				{
					int videortpport=i->port();
					mVideoParams.setRemotePort(videortpport);
				}
			}
		}
		mVideoParams.setRemoteAddr(ipaddr);
#endif
	}

	mInsdp=sdp;

	mParent->msgIncomingCallRingingStart(mCallId);

	mStream->ringstream_start("ring.wav",mOutputDevice);
	mParent->msgIncomingCallRingingStart(mCallId);

	ServerInviteSession *sis = (ServerInviteSession *)(aLegAppDialog->getInviteSession().get());
	sis->provisional(180);

	setCallState(CalleeEarlyOffer);
}

void AppSession::RemoteHold(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)
{
	setCallState(Connected);
	sdp.session().media().front();
	bool holdstate = this->mLine->getHoldState();
	if(!holdstate)
	{
		if(mStream!=NULL)
		{
			if(mStream->isRunning())
			{
				mStream->audiostream_stop();
			}
		}
#ifdef VIDEO_ENABLE
		if(mVideoStream!=NULL)
		{
			if(mVideoStream->isRunning())
				mVideoStream->videostream_stop();
		}
#endif
		mParent->msgNOTIFY("line is hold ");
	}
	else
	{
		Data ipaddr=sdp.session().connection().getAddress();
		int rtpport=sdp.session().media().begin()->port();

		if(ipaddr==Data::Empty)
		{
			ipaddr=sdp.session().origin().getAddress();
		}
		mAudioParams.setRemoteAddr(ipaddr);

		for (list<SdpContents::Session::Medium>::const_iterator i = sdp.session().media().begin(); 
			i != sdp.session().media().end(); ++i)
		{
			if(i->name() == Symbols::audio)
			{
				int rtpport=i->port();
				mAudioParams.setRemotePort(rtpport);

				SdpContents::Session::Medium medium;
				buildCodecs((SdpContents::Session::Medium*)&medium);

				SdpContents::Session::Codec incode=medium.findFirstMatchingCodecs(*i);
				if((incode.getName()==Data::Empty)||(incode.payloadType()==TELEPHONE_EVENT))
				{
					mParent->msgNOTIFY("media not supported ");
					DisconnectCall();
					return;
				}
				int pt=incode.payloadType();
				mInboundCodec=pt;
				mOutboundCodec=pt;
				mAudioParams.setPayType(pt);
			}
#ifdef VIDEO_ENABLE
			else if(i->name() == Symbols::video)
			{
				if(mParent->getDialerConfig().getVideoPlay())
				{
					SdpContents::Session::Medium medium;
					buildVideoCodecs((SdpContents::Session::Medium*)&medium);

					SdpContents::Session::Codec incode=medium.findFirstMatchingCodecs(*i);
					if(incode.getName()==Data::Empty)
					{
						mParent->msgNOTIFY("video media not supported ");
					}
					else
					{
						int videortpport=i->port();
						mVideoParams.setRemotePort(videortpport);
					}
				}
			}
			mVideoParams.setRemoteAddr(ipaddr);
#endif
		}
		mInsdp=sdp;

		if(mStream!=NULL)
		{
			if(!(mStream->isRunning()))
			{
				mStream->create_rtpsession(mRTPRxPort);
				mStream->audiostream_start(mAudioParams.getRemoteAddr().c_str(),
					mAudioParams.getRemotePort(),
					mAudioParams.getPayType(),
					mInputDevice,
					mOutputDevice,
					false,
					false,
					false);
			}
		}
#ifdef VIDEO_ENABLE
		if(mVideoStream!=NULL)
		{
			if(mParent->getDialerConfig().getVideoCapture() ||  mParent->getDialerConfig().getVideoPlay())
			{
				mVideoStream->create_rtpsession(mVideoRxPort);
				mVideoStream->videostream_start(mVideoParams.getRemoteAddr().c_str(),
					mVideoParams.getRemotePort(),
					mVideoParams.getPayType(),
					mParent->getDialerConfig().getVideoCapture(),
					mParent->getDialerConfig().getVideoPlay());
				mParent->msgNOTIFY("video media is created ");
			}
		}
#endif
		mParent->msgNOTIFY("line is unhold ");
	}
	is->provideAnswer(mOutsdp);
	
	this->mLine->setHoldState(!holdstate);
	mParent->msgHoldCall(this->mLine->getLineNo());
	
}

void AppSession::onOfferRequired(InviteSessionHandle sis, const SipMessage& msg)
{
	switch(this->mCallState)
	{
	case Undefined:
	case Connected:
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
		break;
	case CallerEarly:
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:
		break;

	case CalleeStart:
		setCallState(CalleeNoOffer);
		ReceivedNoOffer(sis, msg);
		break;
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::ReceivedNoOffer(InviteSessionHandle is, const SipMessage& msg)
{
	mParent->msgIncomingCallRingingStart(mCallId);

	mStream->ringstream_start("ring.wav",mOutputDevice);
	mParent->msgIncomingCallRingingStart(mCallId);

	ServerInviteSession *sis = (ServerInviteSession *)(is.get());
	sis->provisional(180);

	setCallState(CalleeEarlyNoOffer);
}

void AppSession::onOfferRejected(InviteSessionHandle is, const SipMessage *msg)
{
	switch(this->mCallState)
	{
	case Undefined:
	case Connected:
	case SentUpdate:  
	case SentUpdateGlare:
		break;
	case SentReinvite:
		setCallState(Connected);
		mParent->msgNOTIFY("hold is rejected ");
//		HoldBackToConnected(is);
		break;
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
		break;
	case CallerEarly:
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::onProvisional(ClientInviteSessionHandle cis, const SipMessage& msg)
{
	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
		break;
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
		setCallState(CallerEarly);
		CallResponse(msg);
		break;
	case CallerEarly:
		CallResponse(msg);
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::CallResponse(const SipMessage& msg)
{
	if(mStream==NULL)
	{
		return;
	}
	bool run=mStream->isRunning();
	if(run)
	{
		return;
	}
	if(msg.getContents()==NULL)
	{
		mStream->ringstream_start("call.wav",mOutputDevice);
		mParent->msgNOTIFY("Ring...");
		int status = msg.header(resip::h_StatusLine).statusCode();
		resip::Data reason = msg.header(resip::h_StatusLine).reason();
		mParent->msgProvisionalResponse(mLine->getLineNo(),status,reason);
	}	
}

void AppSession::onEarlyMedia( ClientInviteSessionHandle cis, const SipMessage& msg, const SdpContents& sdp )
{	
	switch(this->mCallState)
	{
	case Undefined:  
	case Connected:
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
		break;
	case CallerEarly:
		ProcessEarlyMedia(msg, sdp);
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::ProcessEarlyMedia( const SipMessage& msg, const SdpContents& sdp)
{
	if(mStream==NULL)
	{
		return;
	}
	bool run=mStream->isRunning();
	if(run)
	{
		return;
	}
	
	SdpContents::Session::Medium medium;
	buildCodecs((SdpContents::Session::Medium*)&medium);
	SdpContents::Session::Codec incode=medium.findFirstMatchingCodecs(*sdp.session().media().begin());
	if(incode.getName()==Data::Empty)
	{
		mParent->msgNOTIFY("media not supported ");
		mParent->msgFailToConnect(mLine->getLineNo());
		return;
	}
	mParent->msgNOTIFY("Ring...early media");
	mStream->create_rtpsession(mRTPRxPort);
	Data ipaddr=sdp.session().connection().getAddress();
	int rtpport=sdp.session().media().begin()->port();
	mStream->audiostream_start(ipaddr.c_str(),
				rtpport,
				incode.payloadType(),
				mInputDevice,
				mOutputDevice,
				false,
				false,
				false);/*don't listen ring back, add it*/
	//mStream->receive_mediastream(incode.payloadType(),mOutputDevice);

}

void AppSession::onAnswer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)
{
	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
		break;
	case SentUpdate:  
	case SentUpdateGlare:
		break;
	case SentReinvite:
		setCallState(Connected);
		HoldCallAnswer(is, msg, sdp);
		break;
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:/*Auto Answer*/
	case CallerEarly:
		setCallState(Connected);
		ReceivedAnswer(is, msg, sdp);
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::ReceivedAnswer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)
{
	InfoLog(<<"received answer");
	//callState = CALL_ANSWERED;
		
	Data ipaddr=sdp.session().connection().getAddress();
	if(ipaddr==Data::Empty)
	{
		ipaddr=sdp.session().origin().getAddress();
	}
	mAudioParams.setRemoteAddr(ipaddr);

	for (list<SdpContents::Session::Medium>::const_iterator i = sdp.session().media().begin(); 
		i != sdp.session().media().end(); ++i)
	{
		if(i->name() == Symbols::audio)
		{
			int rtpport=i->port();
			mAudioParams.setRemotePort(rtpport);

			SdpContents::Session::Medium medium;
			buildCodecs((SdpContents::Session::Medium*)&medium);

			SdpContents::Session::Codec incode=medium.findFirstMatchingCodecs(*i);
			if((incode.getName()==Data::Empty)||(incode.payloadType()==TELEPHONE_EVENT))
			{
				mParent->msgNOTIFY("media not supported ");
				DisconnectCall();
				return;
			}
			int pt=incode.payloadType();
			mInboundCodec=pt;
			mOutboundCodec=pt;
			mAudioParams.setPayType(pt);
		}
#ifdef VIDEO_ENABLE
		else if(i->name() == Symbols::video)
		{
			if(mParent->getDialerConfig().getVideoPlay())
			{
				SdpContents::Session::Medium medium;
				buildVideoCodecs((SdpContents::Session::Medium*)&medium);

				SdpContents::Session::Codec incode=medium.findFirstMatchingCodecs(*i);
				if(incode.getName()==Data::Empty)
				{
					mParent->msgNOTIFY("video media not supported ");
				}
				else
				{
					int videortpport=i->port();
					mVideoParams.setRemotePort(videortpport);
				}
			}
		}
		mVideoParams.setRemoteAddr(ipaddr);
#endif
	}

	mInsdp=sdp;
} 

void AppSession::HoldCallAnswer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp)
{

	bool holdstate = this->mLine->getHoldState();
	if(!holdstate)
	{
		if(mStream!=NULL)
		{
			if(mStream->isRunning())
			{
				mStream->audiostream_stop();
			}
		}
#ifdef VIDEO_ENABLE
		if(mVideoStream!=NULL)
		{
			if(mVideoStream->isRunning())
				mVideoStream->videostream_stop();
		}
#endif
		mParent->msgNOTIFY("line is hold ");
	}
	else
	{
		Data ipaddr=sdp.session().connection().getAddress();
		int rtpport=sdp.session().media().begin()->port();

		if(ipaddr==Data::Empty)
		{
			ipaddr=sdp.session().origin().getAddress();
		}
		mAudioParams.setRemoteAddr(ipaddr);

		for (list<SdpContents::Session::Medium>::const_iterator i = sdp.session().media().begin(); 
			i != sdp.session().media().end(); ++i)
		{
			if(i->name() == Symbols::audio)
			{
				int rtpport=i->port();
				mAudioParams.setRemotePort(rtpport);

				SdpContents::Session::Medium medium;
				buildCodecs((SdpContents::Session::Medium*)&medium);

				SdpContents::Session::Codec incode=medium.findFirstMatchingCodecs(*i);
				if((incode.getName()==Data::Empty)||(incode.payloadType()==TELEPHONE_EVENT))
				{
					mParent->msgNOTIFY("media not supported ");
					DisconnectCall();
					return;
				}
				int pt=incode.payloadType();
				mInboundCodec=pt;
				mOutboundCodec=pt;
				mAudioParams.setPayType(pt);
			}
#ifdef VIDEO_ENABLE
			else if(i->name() == Symbols::video)
			{
				if(mParent->getDialerConfig().getVideoPlay())
				{
					SdpContents::Session::Medium medium;
					buildVideoCodecs((SdpContents::Session::Medium*)&medium);

					SdpContents::Session::Codec incode=medium.findFirstMatchingCodecs(*i);
					if(incode.getName()==Data::Empty)
					{
						mParent->msgNOTIFY("video media not supported ");
					}
					else
					{
						int videortpport=i->port();
						mVideoParams.setRemotePort(videortpport);
					}
				}
			}
			mVideoParams.setRemoteAddr(ipaddr);
#endif
		}
		mInsdp=sdp;

		if(mStream!=NULL)
		{
			if(!(mStream->isRunning()))
			{
				mStream->create_rtpsession(mRTPRxPort);
				mStream->audiostream_start(mAudioParams.getRemoteAddr().c_str(),
					mAudioParams.getRemotePort(),
					mAudioParams.getPayType(),
					mInputDevice,
					mOutputDevice,
					false,
					false,
					false);
			}
		}
#ifdef VIDEO_ENABLE
		if(mVideoStream!=NULL)
		{
			if(mParent->getDialerConfig().getVideoCapture() ||  mParent->getDialerConfig().getVideoPlay())
			{
				mVideoStream->create_rtpsession(mVideoRxPort);
				mVideoStream->videostream_start(mVideoParams.getRemoteAddr().c_str(),
					mVideoParams.getRemotePort(),
					mVideoParams.getPayType(),
					mParent->getDialerConfig().getVideoCapture(),
					mParent->getDialerConfig().getVideoPlay());
			}
		}
#endif
		mParent->msgNOTIFY("line is unhold ");
	}
	
	this->mLine->setHoldState(!holdstate);
	mParent->msgHoldCall(this->mLine->getLineNo());

}

void AppSession::onConnected(InviteSessionHandle is, const SipMessage& msg)
{
	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
		break;
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
	case CallerEarly:
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
		break;
	case CalleeAccepted:
		setCallState(Connected);
		SuccesstoConnected(is, msg);
		break;
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:
		break;
	case CalleeAcceptedWaitingAnswer:
		setCallState(Connected);
		SuccesstoConnected(is, msg);
		break;
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::SuccesstoConnected(InviteSessionHandle is, const SipMessage& msg)
{
	time(&mConnectTime);

   if(mStream!=NULL)
   {
	   mStream->audiostream_stop();
   }
   mStream->create_rtpsession(mRTPRxPort);
   bool ec=mParent->getDialerConfig().getEchoNoiseCancellation();
   int agc=mParent->getDialerConfig().getAGCLevel();
   bool boost=mParent->getDialerConfig().getMicBoost();
   mStream->audiostream_start(mAudioParams.getRemoteAddr().c_str(),
	   mAudioParams.getRemotePort(),
	   mAudioParams.getPayType(),
	   mInputDevice,
	   mOutputDevice,
	   ec,
	   agc,
	   boost);

   mParent->msgSuccessToConnect(mLine->getLineNo(),mAudioParams.getRemoteAddr().c_str(),mAudioParams.getRemotePort());

#ifdef VIDEO_ENABLE
   if(mVideoStream!=NULL)
   {
	   if(mParent->getDialerConfig().getVideoCapture() ||  mParent->getDialerConfig().getVideoPlay())
	   {
		   mVideoStream->create_rtpsession(mVideoRxPort);

		   mVideoStream->videostream_start(mVideoParams.getRemoteAddr().c_str(),
			   mVideoParams.getRemotePort(),
			   mVideoParams.getPayType(),
			   mParent->getDialerConfig().getVideoCapture(),
			   mParent->getDialerConfig().getVideoPlay());
		   mParent->msgNOTIFY("video media is created ");
	   }
   }
#endif

}

void AppSession::onConnected(ClientInviteSessionHandle cis, const SipMessage& msg)
{
	switch(this->mCallState)
	{
	case Undefined:
		break;
	case Connected:
		SuccesstoConnected(cis, msg);
		break;
	case SentUpdate:  
	case SentUpdateGlare:
	case SentReinvite:  
	case SentReinviteGlare:
	case SentReinviteNoOffer:
	case SentReinviteAnswered:
	case SentReinviteNoOfferGlare:
	case ReceivedUpdate:
	case ReceivedReinvite:
	case ReceivedReinviteNoOffer:
	case ReceivedReinviteSentOffer:
	case Answered:
	case WaitingToOffer:
	case WaitingToRequestOffer:
	case WaitingToTerminate:  
	case WaitingToHangup:
	case Terminated:
		break;
	case CallerStart:
		break;
	case CallerEarly:
//		setCallState(Connected);
//		SuccesstoConnected(is, msg);
		break;
	case CallerEarlyWithOffer:
	case CallerEarlyWithAnswer:
	case CallerAnswered:
	case CallerSentUpdateEarly:
	case CallerSentUpdateEarlyGlare:
	case CallerReceivedUpdateEarly:
	case CallerSentAnswer:
	case CallerQueuedUpdate:
	case CallerCancelled:

	case CalleeStart:
	case CalleeOffer: 
	case CalleeOfferProvidedAnswer:
	case CalleeEarlyOffer:
	case CalleeEarlyProvidedAnswer:

	case CalleeNoOffer:
	case CalleeProvidedOffer:
	case CalleeEarlyNoOffer:
	case CalleeEarlyProvidedOffer:
	case CalleeAccepted:
	case CalleeWaitingToOffer:
	case CalleeWaitingToRequestOffer:

	case CalleeAcceptedWaitingAnswer:
	case CalleeReceivedOfferReliable:
	case CalleeNoOfferReliable:
	case CalleeFirstSentOfferReliable:
	case CalleeFirstSentAnswerReliable:
	case CalleeNegotiatedReliable:
	case CalleeSentUpdate:
	case CalleeSentUpdateAccepted:
	case CalleeReceivedUpdate:
	case CalleeReceivedUpdateWaitingAnswer:
	case CalleeWaitingToTerminate:
	case CalleeWaitingToHangup:
		break;
	default:
		break;
	}
}

void AppSession::SuccesstoConnected(ClientInviteSessionHandle cis, const SipMessage& msg)
{
	time(&mConnectTime);

   if(mStream!=NULL)
   {
	   mStream->audiostream_stop();
   }

   mStream->create_rtpsession(mRTPRxPort);
   bool ec=mParent->getDialerConfig().getEchoNoiseCancellation();
   int agc=mParent->getDialerConfig().getAGCLevel();
   bool boost=mParent->getDialerConfig().getMicBoost();
   mStream->audiostream_start(mAudioParams.getRemoteAddr().c_str(),
	   mAudioParams.getRemotePort(),
	   mAudioParams.getPayType(),
	   mInputDevice,
	   mOutputDevice,
	   ec,
	   agc,
	   boost);

   mParent->msgSuccessToConnect(mLine->getLineNo(),mAudioParams.getRemoteAddr().c_str(),mAudioParams.getRemotePort());

#ifdef VIDEO_ENABLE
   if(mVideoStream!=NULL)
   {
	   if(mParent->getDialerConfig().getVideoCapture() ||  mParent->getDialerConfig().getVideoPlay())
	   {
		   mVideoStream->create_rtpsession(mVideoRxPort);

		   mVideoStream->videostream_start(mVideoParams.getRemoteAddr().c_str(),
			   mVideoParams.getRemotePort(),
			   mVideoParams.getPayType(),
			   mParent->getDialerConfig().getVideoCapture(),
			   mParent->getDialerConfig().getVideoPlay());
		   mParent->msgNOTIFY("video media is created ");
	   }
   }
#endif

}

bool AppSession::DigitDTMF(resip::Data sDigit)
{
	return true;
}

bool AppSession::EnableForceInbandDTMF()
{
	return true;
}

bool AppSession::DisableForceInbandDTMF()
{
	return true;
}

bool AppSession::SetTOS(int nValue)
{
	return true;
}

int AppSession::GetTOS()
{
	return 0;
}

int AppSession::GetOutboundCodec()
{
	return mOutboundCodec;
}

int AppSession::GetInboundCodec()
{
	return mInboundCodec;
}

bool AppSession::IsRecording()
{
	return true;
}

bool AppSession::StartRecording(int nRecordVoice, bool bRecordCompress)
{
	return true;
}

bool AppSession::StopRecording()
{
	return true;
}

bool AppSession::ResetRecording()
{
	return true;
}

bool AppSession::SaveRecordingToWaveFile(resip::Data sFileName)
{
	return true;
}

bool AppSession::IsWaveFilePlaying()
{
	return true;
}

bool AppSession::PlayWaveOpen(resip::Data sFileName)
{
	return true;
}

bool AppSession::PlayWaveSkipTo(int nSeconds)
{
	return true;
}

int AppSession::PlayWaveTotalTime()
{
	return 0;
}

bool AppSession::PlayWavePause()
{
	return true;
}

bool AppSession::PlayWaveStart(bool bListen)
{
	return true;
}

bool AppSession::PlayWaveStop()
{
	return true;
}

int AppSession::PlayWavePosition()
{
	return 0;
}

void AppSession::initialCommon()
{
	
//	if(mParent->getDialerConfig().getFirewallPolicy()==DialerConfiguration::USE_STUN)
	{
		bool flag=runStunTests();
	}
}

void AppSession::buildSdpTemplate(resip::SdpContents *sdp)
{
	sdp->session().version()=0;
    sdp->session().name()="a-site";
	sdp->session().origin()=SdpContents::Session::Origin("site", 
                                                             1234567,
                                                             1234567,
                                                             SdpContents::IP4,
                                                             getMediaAddr());
    sdp->session().addTime(SdpContents::Session::Time(0,0));
    sdp->session().connection()=SdpContents::Session::Connection(SdpContents::IP4, getMediaAddr(), 0);
}

resip::Data AppSession::getMediaAddr()
{
	if(mParent->getDialerConfig().getOutboundProxy()!=Data::Empty)
	{
		return mParent->getDialerConfig().getOutboundProxy();		
	}
	else if(mAudioParams.getNatPort()>0)
	{
		return mAudioParams.getNatAddr();
	}
	else
	{
		return mParent->getDialerConfig().getLocRealm();
	}
}

void AppSession::buildCodecs(SdpContents::Session::Medium* media)
{
//	std::list<resip::SdpContents::Session::Codec>::const_iterator sIter;
//	for (sIter=mParent->getDialerConfig().mCodecs.begin(); sIter!=mParent->getDialerConfig().mCodecs.end(); ++sIter)
//	{
//		media->addCodec(*sIter);
//	}
	int codec = mParent->getDialerConfig().getCodecPriority();
	switch (codec)
	{
	case PCMU:
		if(mParent->getDialerConfig().getCodecPCMU())
		{
			media->addCodec(SdpContents::Session::Codec::ULaw_8000);
		}
		if(mParent->getDialerConfig().getCodecPCMA())
		{
			media->addCodec(SdpContents::Session::Codec::ALaw_8000);
		}
		if(mParent->getDialerConfig().getCodecG729())
		{
			media->addCodec(SdpContents::Session::Codec::G729_8000);
		}
		break;
	case G729:
		if(mParent->getDialerConfig().getCodecG729())
		{
			media->addCodec(SdpContents::Session::Codec::G729_8000);
		}
		if(mParent->getDialerConfig().getCodecPCMA())
		{
			media->addCodec(SdpContents::Session::Codec::ALaw_8000);
		}
		if(mParent->getDialerConfig().getCodecPCMU())
		{
			media->addCodec(SdpContents::Session::Codec::ULaw_8000);
		}
		break;
	default:		
		if(mParent->getDialerConfig().getCodecPCMA())
		{
			media->addCodec(SdpContents::Session::Codec::ALaw_8000);
		}
		if(mParent->getDialerConfig().getCodecPCMU())
		{
			media->addCodec(SdpContents::Session::Codec::ULaw_8000);
		}
		if(mParent->getDialerConfig().getCodecG729())
		{
			media->addCodec(SdpContents::Session::Codec::G729_8000);
		}
		break;
	}
	
	if(mParent->getDialerConfig().getCodecG723())
	{
		media->addCodec(SdpContents::Session::Codec::G723_8000);
	}
	if(mParent->getDialerConfig().getCodecGSM())
	{
		media->addCodec(SdpContents::Session::Codec::GSM_8000);
	}
	if(mParent->getDialerConfig().getCodeciLBC())
	{
		media->addCodec(SdpContents::Session::Codec::iLBC_8000);
		media->addAttribute("fmtp", "97 mode=30");
	}

}

#ifdef VIDEO_ENABLE
void AppSession::buildVideoCodecs(SdpContents::Session::Medium* media)
{
	if(mParent->getDialerConfig().getCodecH263_1998())
	{
		media->addCodec(SdpContents::Session::Codec::H263v2_90000);
		media->addAttribute("fmtp", "98 CIF=1;QCIF=1");
	}
}
#endif

int AppSession::parse_stunserver_addr(const resip::Uri server, struct sockaddr_storage *ss, socklen_t *socklen)
{
	struct addrinfo hints,*res=NULL;
	int ret;

	memset(&hints,0,sizeof(hints));
	hints.ai_family=PF_INET;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=IPPROTO_UDP;
	ret=getaddrinfo(server.host().data(),Data(server.port()).data(),&hints,&res);
	if (ret!=0){
		return -1;
	}
	if (!res) return -1;
	memcpy(ss,res->ai_addr,res->ai_addrlen);
	*socklen=res->ai_addrlen;
	freeaddrinfo(res);
	return 0;
}


SOCKET AppSession::create_socket(int local_port)
{
	struct sockaddr_in laddr;
	SOCKET sock;
	int optval;
	sock=::socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (sock<0)
	{
		ErrLog(<<"Fail to create socket");
		return -1;
	}
	memset (&laddr,0,sizeof(laddr));
	laddr.sin_family=AF_INET;
	laddr.sin_addr.s_addr=INADDR_ANY;
	laddr.sin_port=htons(local_port);
	if (::bind(sock,(struct sockaddr*)&laddr,sizeof(laddr))<0)
	{
		ErrLog(<<"Bind socket to 0.0.0.0");
		close(sock);
		return -1;
	}
	optval=1;
	if (::setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
				(char*)&optval, sizeof (optval))<0)
	{
		WarningLog(<<"Fail to set SO_REUSEADDR");
	}

#if	!defined(_WIN32) && !defined(_WIN32_WCE)
	return fcntl (sock, F_SETFL, O_NONBLOCK);
#else
	unsigned long nonBlock = 1;
	ioctlsocket(sock, FIONBIO , &nonBlock);
#endif
	return sock;
}

int AppSession::recvStunResponse(SOCKET sock, Tuple &mStunMappedAddress)
{
	char buf[STUN_MAX_MESSAGE_SIZE];
   	int len = STUN_MAX_MESSAGE_SIZE;
	StunMessage resp;
	len=::recv(sock,buf,len,0);
	if (len>0)
	{
		in_addr sin_addr;
		stunParseMessage(buf,len, resp,false );
#if defined(WIN32)
		sin_addr.S_un.S_addr = htonl(resp.mappedAddress.ipv4.addr);
#else
		sin_addr.s_addr = htonl(resp.mappedAddress.ipv4.addr);
#endif
		mStunMappedAddress = Tuple(sin_addr,resp.mappedAddress.ipv4.port, UDP);
	}
	return len;
}

int AppSession::sendStunRequest(int sock, const struct sockaddr *server, socklen_t addrlen, int id)
{
	char buf[STUN_MAX_MESSAGE_SIZE];
	int len = STUN_MAX_MESSAGE_SIZE;
	StunAtrString username;
   	StunAtrString password;
	StunMessage req;
	int err;
	memset(&req, 0, sizeof(StunMessage));
	memset(&username,0,sizeof(username));
	memset(&password,0,sizeof(password));
	stunBuildReqSimple( &req, username, false , false , id);
	len = stunEncodeMessage( req, buf, len, password, false);
	if (len<=0)
	{
		ErrLog(<<"Fail to encode stun message.");
		return -1;
	}
	err=::sendto(sock,buf,len,0,server,addrlen);
	if (err<0)
	{
		ErrLog(<<"sendto failed: ");
		return -1;
	}
	return 0;
}

bool AppSession::runStunTests()
{
	const Uri server=mParent->getDialerConfig().getStunIdentity();
	bool got_audio=false;
	bool got_video=false;
		
	if (server.host()!=Data::Empty)
	{
		struct sockaddr_storage ss;
		socklen_t ss_len;
		SOCKET sock1=-1;
		SOCKET sock2=-1;

		struct timeval init,cur;
		if (parse_stunserver_addr(server,&ss,&ss_len)<0)
		{
			ErrLog(<<"Fail to parser stun server address: "<<server);
			return got_audio;
		}

		mParent->msgNOTIFY("run stun test...");
		sock1=create_socket(mParent->getDialerConfig().getRTPPort().convertInt());
		if (sock1<0) return got_audio;

#ifdef VIDEO_ENABLE
		sock2=create_socket(mParent->getDialerConfig().getVideoRTPPort().convertInt());
		if (sock2<0) return got_video;
#endif
		sendStunRequest(sock1,(struct sockaddr*)&ss,ss_len,1);

#ifdef VIDEO_ENABLE
		sendStunRequest(sock2,(struct sockaddr*)&ss,ss_len,1);
#endif
		gettimeofday(&init,NULL);

		do{
			double elapsed;
#ifdef WIN32
			Sleep(10);
#else
			usleep(10000);
#endif

			Tuple stunaddr1;
			if (recvStunResponse(sock1,stunaddr1)>0)
			{
				mParent->msgNOTIFY("get STUN response ");
				Data addr=Tuple::inet_ntop(stunaddr1);
				;
				mAudioParams.setNatAddr(addr);
				mAudioParams.setNatPort(stunaddr1.getPort());
				got_audio=true;
			}
#ifdef VIDEO_ENABLE
			Tuple stunaddr2;
			if (recvStunResponse(sock2,stunaddr2)>0)
			{
				mParent->msgNOTIFY("get STUN response ");
				Data addr=Tuple::inet_ntop(stunaddr2);
				;
				mVideoParams.setNatAddr(addr);
				mVideoParams.setNatPort(stunaddr2.getPort());
				got_video=true;
			}
#endif
			gettimeofday(&cur,NULL);
			elapsed=((cur.tv_sec-init.tv_sec)*1000.0)+((cur.tv_usec-init.tv_usec)/1000.0);
			if (elapsed>2000)
			{
				break;
			}

		}while( (!got_audio) 
#ifdef VIDEO_ENABLE
			&& (!got_video) 
#endif
			);

		if (!got_audio)
		{
			mParent->msgNOTIFY("No stun server response ");
		}
#ifdef VIDEO_ENABLE
		if (!got_video)
		{
			mParent->msgNOTIFY("No stun server response ");
		}
#endif

#if	!defined(_WIN32) && !defined(_WIN32_WCE)
		::close (sock1);
#ifdef VIDEO_ENABLE
		::close (sock2);
#endif
#else
		::closesocket(sock1);
#ifdef VIDEO_ENABLE
		::closesocket(sock2);
#endif
#endif
	}
	return got_audio;
}

int AppSession::getRTPRxPort() const
{
	return mRTPRxPort;
}

PhoneLine* AppSession::getPhoneLine() const
{
	return mLine;
}

DialInstance *AppSession::getParent() const
{
	return mParent;
}

