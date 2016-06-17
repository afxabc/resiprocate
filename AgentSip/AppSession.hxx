
#ifndef APPSESSION_H
#define APPSESSION_H

#include <string>
#include "rutil/Data.hxx"
#include "resip/stack/SdpContents.hxx"
#include "resip/dum/Handles.hxx"
#include "resip/dum/InviteSession.hxx"
#include "resip/dum/InviteSessionHandler.hxx"

#include "StreamParam.hxx"
#include "resip/dum/DialogUsageManager.hxx"

class PhoneLine;
class DialInstance;
class MyAppDialogSet;
class MyAppDialog;
class MyAudioStream;
#ifdef VIDEO_ENABLE
class MyVideoStream;
#endif

using namespace resip;

class AppSession
{
public:

	typedef enum 
	{
         Undefined,                 // Not used
         Connected,
         SentUpdate,                // Sent an UPDATE
         SentUpdateGlare,           // got a 491
         SentReinvite,              // Sent a reINVITE
         SentReinviteGlare,         // Got a 491
         SentReinviteNoOffer,       // Sent a reINVITE with no offer (requestOffer)
         SentReinviteAnswered,      // Sent a reINVITE no offer and received a 200-offer
         SentReinviteNoOfferGlare,  // Got a 491
         ReceivedUpdate,            // Received an UPDATE
         ReceivedReinvite,          // Received a reINVITE
         ReceivedReinviteNoOffer,   // Received a reINVITE with no offer
         ReceivedReinviteSentOffer, // Sent a 200 to a reINVITE with no offer
         Answered,
         WaitingToOffer,
         WaitingToRequestOffer,
         WaitingToTerminate,        // Waiting for 2xx response before sending BYE
         WaitingToHangup,           // Waiting for ACK before sending BYE
         Terminated,                // Ended. waiting to delete
		 TermKill,

         CallerStart,
         CallerEarly,
         CallerEarlyWithOffer,
         CallerEarlyWithAnswer,
         CallerAnswered,
         CallerSentUpdateEarly,
         CallerSentUpdateEarlyGlare,
         CallerReceivedUpdateEarly,
         CallerSentAnswer,
         CallerQueuedUpdate,
         CallerCancelled,

         CalleeStart,
         CalleeOffer, 
         CalleeOfferProvidedAnswer,
         CalleeEarlyOffer,
         CalleeEarlyProvidedAnswer, 

         CalleeNoOffer, 
         CalleeProvidedOffer, 
         CalleeEarlyNoOffer, 
         CalleeEarlyProvidedOffer, 
         CalleeAccepted, 
         CalleeWaitingToOffer, 
         CalleeWaitingToRequestOffer, 

         CalleeAcceptedWaitingAnswer, 
         CalleeReceivedOfferReliable,
         CalleeNoOfferReliable,
         CalleeFirstSentOfferReliable,
         CalleeFirstSentAnswerReliable,
         CalleeNegotiatedReliable,
         CalleeSentUpdate,
         CalleeSentUpdateAccepted,
         CalleeReceivedUpdate,
         CalleeReceivedUpdateWaitingAnswer,
         CalleeWaitingToTerminate,
         CalleeWaitingToHangup

	}CallState;

	static resip::Data callStateNames[];

	static const char *basicClearingReasonName[];
	static unsigned int T0;

	typedef enum 
	{
		Unset,

		// No attempted
		InvalidDestination,
		AuthError,

		// not answered
		NoAnswerCancel,
		NoAnswerTimeout,
		NoAnswerError,	// media negotiation failed

		// Rejected
		RejectBusy,
		RejectOther
	} FullClearingReason;

	// More basic then CallState, used for reporting
	typedef enum
	{
		PreDial,			// the call hasn't started dialing
		Dialing,			// the call is dialing
		Ringing,
		Talking,			// the call has connected
		Finishing,			// the call has been hung up
		Unknown			// unknown
	} CallStatus;

	typedef enum
	{
		NoTransfer,
		Transferor,
		Transferee,
		TransferTarget
	}TransferState;

	AppSession(DialInstance *dial,
		DialogUsageManager* dum, 
		MyAppDialog *aAppDialog,
		const resip::NameAddr& sourceAddr, 
		const resip::Uri& destinationAddr,
		const resip::Data callId);

	AppSession(DialInstance *dial,
		DialogUsageManager* dum,  
		const resip::NameAddr& sourceAddr, 
		const resip::Uri& destinationAddr,
		PhoneLine* line,
		int inputDevice,
		int outputDevice);

	AppSession(const AppSession& session);

	~AppSession();

	class AppMessage
	{
	public:
		typedef enum
		{
			OutCallMsg,
			SendAnswerMsg,
			SendRejectMsg,
			SendHangupMsg,
			HoldLineMsg,
			UnholdLineMsg,
			TransferCallMsg
		}AppMsgType;

	public:
		AppMessage(AppMsgType type,
			int line = 0,
			resip::Data id = resip::Data::Empty,
			resip::Data Uri = resip::Data::Empty,
			int Idev = 0,
			int Odev = 0):Type(type),
							Line(line),
							Callid(id),
							ToUri(Uri),
							Indev(Idev),
							Outdev(Odev)
		{};

	public:
		AppMsgType Type;
		int Line;
		resip::Data Callid;
		resip::Data ToUri;
		int Indev;
		int Outdev;

	};

	bool setCallState(CallState newCallState);
	const resip::Data& getCallStateName(CallState s);
	void setBLegAppDialog(MyAppDialog *myAppDialog);
	void releaseAppDialog(MyAppDialog *myAppDialog);
	void releaseAppDialogSet(MyAppDialogSet *myAppDialogSet);
	void checkProgress(time_t now, bool stopping);
	void checkMessage();

	bool isComplete() ;
	CallStatus getStatus();
	void doOutgoingCall();
	bool Disconnect();
	void DisconnectCall();
	void doHangup();
	bool CallAccept( int nInputDeviceId, int nOutputDeviceId);
	void AcceptOfferCall();
	void AcceptNoOfferCall();
	bool CallTransfer(resip::Data sToURI);
	void InviteFromRefer(InviteSessionHandle is, ServerSubscriptionHandle ssh, const SipMessage& msg);
	bool CallReject();

	bool LineHold();
	void HoldtheLine();
	void RemoteHold(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp);
	bool LineUnHold();
	void UnHoldtheLine();
	void onOutgoingSession( ClientInviteSessionHandle cis, InviteSession::OfferAnswerType oat, const SipMessage& msg);
	void onNewIncomingCall(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg);
	void RecivedNewInCall(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg);

	void onOffer(InviteSessionHandle is, const resip::SipMessage& msg, const resip::SdpContents& isdp);
	void ReceivedOffer(InviteSessionHandle is, const resip::SipMessage& msg, const resip::SdpContents& sdp);
	void onOfferRequired(InviteSessionHandle sis, const SipMessage& msg);
	void ReceivedNoOffer(InviteSessionHandle sis, const SipMessage& msg);
	void onOfferRejected(InviteSessionHandle is, const SipMessage *msg);
	void onProvisional(ClientInviteSessionHandle cis, const SipMessage& msg);
	void CallResponse(const SipMessage& msg);
	void onEarlyMedia( ClientInviteSessionHandle cis, const SipMessage& msg, const SdpContents& sdp );
	void ProcessEarlyMedia( const SipMessage& msg, const SdpContents& sdp);
	void onAnswer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp);
	void ReceivedAnswer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp);
	void HoldCallAnswer(InviteSessionHandle is, const SipMessage& msg, const SdpContents& sdp);
	void onConnected(InviteSessionHandle is, const SipMessage& msg);
	void SuccesstoConnected(InviteSessionHandle is, const SipMessage& msg);
	void onConnected(ClientInviteSessionHandle cis, const SipMessage& msg);
	void SuccesstoConnected(ClientInviteSessionHandle cis, const SipMessage& msg);
	void onFailure(ClientInviteSessionHandle cis, const SipMessage& msg);
	void onTerminated(InviteSessionHandle is, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg);

	void onRefer(InviteSessionHandle is, ServerSubscriptionHandle ssh, const SipMessage& msg);
	void ReciveTransfer(InviteSessionHandle is, ServerSubscriptionHandle ssh, const SipMessage& msg);


	void onStopping();
	bool runStunTests();
	int parse_stunserver_addr(const resip::Uri server, struct sockaddr_storage *ss, socklen_t *socklen);
	SOCKET create_socket(int local_port);
	int recvStunResponse(SOCKET sock, Tuple &mStunMappedAddress);
	int sendStunRequest(int sock, const struct sockaddr *server, socklen_t addrlen, int id);

	bool DigitDTMF(resip::Data sDigit);
	bool EnableForceInbandDTMF();
	bool DisableForceInbandDTMF();

	bool SetTOS(int nValue);
	int GetTOS();
	int GetOutboundCodec();
	int GetInboundCodec();

	bool IsRecording();
	bool StartRecording(int nRecordVoice, bool bRecordCompress);
	bool StopRecording();
	bool ResetRecording();
	bool SaveRecordingToWaveFile(resip::Data sFileName);

	bool IsWaveFilePlaying();
	bool PlayWaveOpen(resip::Data sFileName);
	bool PlayWaveSkipTo(int nSeconds);
	int PlayWaveTotalTime();
	bool PlayWavePause();
	bool PlayWaveStart(bool bListen);
	bool PlayWaveStop();
	int PlayWavePosition();
	
	void initialCommon();
	void buildSdpTemplate(resip::SdpContents *sdp);
	resip::Data getMediaAddr();
	void buildCodecs(resip::SdpContents::Session::Medium* media);

#ifdef VIDEO_ENABLE
	void buildVideoCodecs(resip::SdpContents::Session::Medium* media);
#endif

	void setCallId(resip::Data did) {mCallId=did;};
	resip::Data getCallId() {return mCallId;};

	void setRTPRxIP(resip::Data ip) {mRTPRxIP=ip;};
	resip::Data getRTPRxIP() {return mRTPRxIP;};

	void setRTPRxPort(int port) {mRTPRxPort=port;};
	int getRTPRxPort() const;
	void setPhoneLine(PhoneLine* line) {mLine=line;};
	PhoneLine* getPhoneLine() const;
	DialInstance *getParent() const;
	void setRemoteTransferState(const TransferState state) {mRemotePoint = state; };

private:

	// Attributes of the call, from the original INVITE
	PhoneLine* mLine;

	resip::NameAddr mSourceAddr;
	resip::Uri mDestinationAddr;

	int mInputDevice;
	int mOutputDevice;
	resip::Data mCallId;
	resip::Data mRTPRxIP;
	int mRTPRxPort;
	
	int mInboundCodec;
	int mOutboundCodec;
	int mTOSValue;

	bool mInbandDTMF;

	bool mLineRecording;
	bool mFilePlaying;
	int mPlayPos;

	//int callState;
	CallState mCallState;
	TransferState mRemotePoint;

//	BasicClearingReason mBClearingReason;
	FullClearingReason mFClearingReason;
	int mRejectOtherCode;

	// Call time information
	time_t mStartTime;
	time_t mConnectTime;
	time_t mFinishTime;

	time_t mWaitDisTime;

	// If we are waiting for something, this is the timeout
	time_t timeout;

	MyAppDialog *aLegAppDialog;
	MyAppDialog *bLegAppDialog;
	MyAppDialogSet *bLegAppDialogSet;

	std::list<AppMessage*> mMessageList;

	int mFailureStatusCode;
	resip::Data *mFailureReason;

	MyAudioStream *mStream;
	StreamParam mAudioParams;
	SdpContents mInsdp;
	SdpContents mOutsdp;

#ifdef VIDEO_ENABLE
	MyVideoStream *mVideoStream;
	int mVideoRxPort;
	StreamParam mVideoParams;
#endif

	resip::DialogUsageManager *mDum;
	DialInstance *mParent;

	friend class DialInstance;
	friend class MyAudioStream;
};

#endif
