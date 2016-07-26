#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rutil/Log.hxx>
#include <rutil/Logger.hxx>
#include <rutil/DnsUtil.hxx>
#include <rutil/MD5Stream.hxx>
#include <rutil/FdPoll.hxx>
#include <resip/stack/SdpContents.hxx>
#include <resip/stack/PlainContents.hxx>
#include <resip/stack/ConnectionTerminated.hxx>
#include <resip/stack/Helper.hxx>
#include <resip/dum/AppDialogSetFactory.hxx>
#include <resip/dum/ClientAuthManager.hxx>
#include <resip/dum/KeepAliveManager.hxx>
#include <resip/dum/ClientInviteSession.hxx>
#include <resip/dum/ServerInviteSession.hxx>
#include <resip/dum/ClientSubscription.hxx>
#include <resip/dum/ServerSubscription.hxx>
#include <resip/dum/ClientRegistration.hxx>
#include <resip/dum/ServerRegistration.hxx>
#include <resip/dum/ServerOutOfDialogReq.hxx>
#include <resip/dum/ClientPagerMessage.hxx>
#include <resip/dum/ServerPagerMessage.hxx>
#include <resip/stack/SipMessage.hxx>
#include <rutil/dns/AresDns.hxx>

#if defined (USE_SSL)
#if defined(WIN32) 
#include "resip/stack/ssl/WinSecurity.hxx"
#else
#include "resip/stack/ssl/Security.hxx"
#endif
#endif

#include "basicClientUserAgent.hxx"
#include "basicClientCall.hxx"

using namespace resip;
using namespace std;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

static unsigned int MaxRegistrationRetryTime = 1800;              // RFC5626 section 4.5 default
static unsigned int BaseRegistrationRetryTimeAllFlowsFailed = 30; // RFC5626 section 4.5 default
//static unsigned int BaseRegistrationRetryTime = 90;               // RFC5626 section 4.5 default
static unsigned int NotifySendTime = 30;  // If someone subscribes to our test event package, then send notifies every 30 seconds
static unsigned int FailedSubscriptionRetryTime = 60; 

//#define TEST_PASSING_A1_HASH_FOR_PASSWORD
#define ONE_CALL_PERTIME

namespace resip
{
class ClientAppDialogSetFactory : public AppDialogSetFactory
{
public:
   ClientAppDialogSetFactory(BasicClientUserAgent& ua) : mUserAgent(ua) {}
   resip::AppDialogSet* createAppDialogSet(DialogUsageManager& dum, const SipMessage& msg)
   {
      switch(msg.method())
      {
         case INVITE:
            return new BasicClientCall(mUserAgent);
            break;
         default:         
            return AppDialogSetFactory::createAppDialogSet(dum, msg); 
            break;
      }
   }
private:
   BasicClientUserAgent& mUserAgent;
};

// Used to set the IP Address in outbound SDP to match the IP address choosen by the stack to send the message on
class SdpMessageDecorator : public MessageDecorator
{
public:
   virtual ~SdpMessageDecorator() {}
   virtual void decorateMessage(SipMessage &msg, 
                                const Tuple &source,
                                const Tuple &destination,
                                const Data& sigcompId)
   {
      SdpContents* sdp = dynamic_cast<SdpContents*>(msg.getContents());
      if(sdp)  
      {
         // Fill in IP and Port from source
 //        sdp->session().connection().setAddress(Tuple::inet_ntop(source), source.ipVersion() == V6 ? SdpContents::IP6 : SdpContents::IP4);
//         sdp->session().origin().setAddress(Tuple::inet_ntop(source), source.ipVersion() == V6 ? SdpContents::IP6 : SdpContents::IP4);
         InfoLog( << "SdpMessageDecorator: src=" << source << ", dest=" << destination << ", msg=" << endl << msg.brief());
      }
   }
   virtual void rollbackMessage(SipMessage& msg) {}  // Nothing to do
   virtual MessageDecorator* clone() const { return new SdpMessageDecorator; }
};

class NotifyTimer : public resip::DumCommand
{
   public:
      NotifyTimer(BasicClientUserAgent& userAgent, unsigned int timerId) : mUserAgent(userAgent), mTimerId(timerId) {}
      NotifyTimer(const NotifyTimer& rhs) : mUserAgent(rhs.mUserAgent), mTimerId(rhs.mTimerId) {}
      ~NotifyTimer() {}

      void executeCommand() { mUserAgent.onNotifyTimeout(mTimerId); }

      resip::Message* clone() const { return new NotifyTimer(*this); }
      EncodeStream& encode(EncodeStream& strm) const { strm << "NotifyTimer: id=" << mTimerId; return strm; }
      EncodeStream& encodeBrief(EncodeStream& strm) const { return encode(strm); }

   private:
      BasicClientUserAgent& mUserAgent;
      unsigned int mTimerId;
};
} // end namespace

BasicClientUserAgent::BasicClientUserAgent() :
	mSipPort(0),
	mRegisterDuration(3600),
	mTransport(NULL),
	mSecurity(NULL),
	mOutboundEnabled(false),
	mProfile(new MasterProfile),
	mPollGrp(FdPollGrp::create()),  // Will create EPoll implementation if available, otherwise FdPoll
	mInterruptor(new EventThreadInterruptor(*mPollGrp)),
	mStack(new SipStack(mSecurity, DnsStub::EmptyNameserverList, mInterruptor, false, 0, 0, mPollGrp)),
	mStackThread(new EventStackThread(*mStack, *mInterruptor, *mPollGrp)),
	mDum(new DialogUsageManager(*mStack)),
	mDumShutdownRequested(false),
	mDumShutdown(false),
	mRegistrationRetryDelayTime(0),
	mCurrentNotifyTimerId(0),
	mRegReport(false)
{

	// Install Managers
	mDum->setClientAuthManager(std::auto_ptr<ClientAuthManager>(new ClientAuthManager));
	mDum->setKeepAliveManager(std::auto_ptr<KeepAliveManager>(new KeepAliveManager));
	mProfile->setKeepAliveTimeForDatagram(30);
	mProfile->setKeepAliveTimeForStream(120);

	// Install Handlers
	mDum->setInviteSessionHandler(this);
	mDum->setDialogSetHandler(this);
	mDum->addOutOfDialogHandler(OPTIONS, this);
	//mDum->addOutOfDialogHandler(REFER, this);
	mDum->setRedirectHandler(this);
	mDum->setClientRegistrationHandler(this);
	mDum->addClientSubscriptionHandler("basicClientTest", this);   // fabricated test event package
	mDum->addServerSubscriptionHandler("basicClientTest", this);

	mDum->setClientPagerMessageHandler(this);
	mDum->setServerPagerMessageHandler(this);

	// Set AppDialogSetFactory
	auto_ptr<AppDialogSetFactory> dsf(new ClientAppDialogSetFactory(*this));
	mDum->setAppDialogSetFactory(dsf);

	mDum->setMasterProfile(mProfile);

	mDum->registerForConnectionTermination(this);
}

BasicClientUserAgent::~BasicClientUserAgent()
{
	stop();

	delete mDum;
	delete mStack;
	delete mStackThread;
	delete mInterruptor;
	delete mPollGrp;
   // Note:  mStack descructor will delete mSecurity
}

void resip::BasicClientUserAgent::setupProfile()
{
	// Supported Methods
	mProfile->clearSupportedMethods();
	mProfile->addSupportedMethod(INVITE);
	mProfile->addSupportedMethod(ACK);
	mProfile->addSupportedMethod(CANCEL);
	mProfile->addSupportedMethod(OPTIONS);
	mProfile->addSupportedMethod(BYE);
	//mProfile->addSupportedMethod(REFER);
	mProfile->addSupportedMethod(NOTIFY);
	mProfile->addSupportedMethod(SUBSCRIBE);
	mProfile->addSupportedMethod(UPDATE);
	mProfile->addSupportedMethod(INFO);
	mProfile->addSupportedMethod(MESSAGE);
	mProfile->addSupportedMethod(PRACK);
	//mProfile->addSupportedOptionTag(Token(Symbols::C100rel));  // Automatically added when using setUacReliableProvisionalMode
	mProfile->setUacReliableProvisionalMode(MasterProfile::Supported);
	mProfile->setUasReliableProvisionalMode(MasterProfile::SupportedEssential);

	// Support Languages
	mProfile->clearSupportedLanguages();
	mProfile->addSupportedLanguage(Token("en"));

	// Support Mime Types
	mProfile->clearSupportedMimeTypes();
	mProfile->addSupportedMimeType(INVITE, Mime("application", "sdp"));
	mProfile->addSupportedMimeType(INVITE, Mime("multipart", "mixed"));
	mProfile->addSupportedMimeType(INVITE, Mime("multipart", "signed"));
	mProfile->addSupportedMimeType(INVITE, Mime("multipart", "alternative"));
	mProfile->addSupportedMimeType(OPTIONS, Mime("application", "sdp"));
	mProfile->addSupportedMimeType(OPTIONS, Mime("multipart", "mixed"));
	mProfile->addSupportedMimeType(OPTIONS, Mime("multipart", "signed"));
	mProfile->addSupportedMimeType(OPTIONS, Mime("multipart", "alternative"));
	mProfile->addSupportedMimeType(PRACK, Mime("application", "sdp"));
	mProfile->addSupportedMimeType(PRACK, Mime("multipart", "mixed"));
	mProfile->addSupportedMimeType(PRACK, Mime("multipart", "signed"));
	mProfile->addSupportedMimeType(PRACK, Mime("multipart", "alternative"));
	mProfile->addSupportedMimeType(UPDATE, Mime("application", "sdp"));
	mProfile->addSupportedMimeType(UPDATE, Mime("multipart", "mixed"));
	mProfile->addSupportedMimeType(UPDATE, Mime("multipart", "signed"));
	mProfile->addSupportedMimeType(UPDATE, Mime("multipart", "alternative"));
	mProfile->addSupportedMimeType(MESSAGE, Mime("text", "plain")); // Invite session in-dialog routing testing
	mProfile->addSupportedMimeType(NOTIFY, Mime("text", "plain"));  // subscription testing
																	//mProfile->addSupportedMimeType(NOTIFY, Mime("message", "sipfrag"));  

																	// Supported Options Tags
	mProfile->clearSupportedOptionTags();
	//mMasterProfile->addSupportedOptionTag(Token(Symbols::Replaces));      
	mProfile->addSupportedOptionTag(Token(Symbols::Timer));     // Enable Session Timers
	if (mOutboundEnabled)
	{
		mProfile->addSupportedOptionTag(Token(Symbols::Outbound));  // RFC 5626 - outbound
		mProfile->addSupportedOptionTag(Token(Symbols::Path));      // RFC 3327 - path
	}
	//mMasterProfile->addSupportedOptionTag(Token(Symbols::NoReferSub));
	//mMasterProfile->addSupportedOptionTag(Token(Symbols::TargetDialog));

	// Supported Schemes
	mProfile->clearSupportedSchemes();
	mProfile->addSupportedScheme("sip");

	// Validation Settings
	mProfile->validateContentEnabled() = false;
	mProfile->validateContentLanguageEnabled() = false;
	mProfile->validateAcceptEnabled() = false;

	// Have stack add Allow/Supported/Accept headers to INVITE dialog establishment messages
	mProfile->clearAdvertisedCapabilities(); // Remove Profile Defaults, then add our preferences
	mProfile->addAdvertisedCapability(Headers::Allow);
	//mProfile->addAdvertisedCapability(Headers::AcceptEncoding);  // This can be misleading - it might specify what is expected in response
	mProfile->addAdvertisedCapability(Headers::AcceptLanguage);
	mProfile->addAdvertisedCapability(Headers::Supported);
	mProfile->setMethodsParamEnabled(true);

	// Install Sdp Message Decorator
	SharedPtr<MessageDecorator> outboundDecorator(new SdpMessageDecorator);
	mProfile->setOutboundDecorator(outboundDecorator);

	// Other Profile Settings
	mProfile->setUserAgent("basicClient/1.0");
	mProfile->setDefaultRegistrationTime(mRegisterDuration);
	mProfile->setDefaultRegistrationRetryTime(120);
	/*
	if (!mContact.host().empty())
	{
		mProfile->setOverrideHostAndPort(mContact);
	}
	*/
	if (!mOutboundProxy.host().empty())
	{
		mProfile->setOutboundProxy(Uri(mOutboundProxy));
		//mProfile->setForceOutboundProxyOnAllRequestsEnabled(true);
		mProfile->setExpressOutboundAsRouteSetEnabled(true);
	}

	// Generate InstanceId appropriate for testing only.  Should be UUID that persists 
	// across machine re-starts and is unique to this applicaiton instance.  The one used 
	// here is only as unique as the hostname of this machine.  If someone runs two 
	// instances of this application on the same host for the same Aor, then things will 
	// break.  See RFC5626 section 4.1
	Data hostname = DnsUtil::getLocalHostName();
	Data instanceHash = hostname.md5().uppercase();
	assert(instanceHash.size() == 32);
	Data instanceId(48, Data::Preallocate);
	instanceId += "<urn:uuid:";
	instanceId += instanceHash.substr(0, 8);
	instanceId += "-";
	instanceId += instanceHash.substr(8, 4);
	instanceId += "-";
	instanceId += instanceHash.substr(12, 4);
	instanceId += "-";
	instanceId += instanceHash.substr(16, 4);
	instanceId += "-";
	instanceId += instanceHash.substr(20, 12);
	instanceId += ">";
	mProfile->setInstanceId(instanceId);
	if (mOutboundEnabled)
	{
		mProfile->setRegId(1);
		mProfile->clientOutboundEnabled() = true;
	}

}

void resip::BasicClientUserAgent::thread()
{
	std::cout << std::endl;
	std::cout << "** sip host : " << mSipHost << std::endl;
	std::cout << "** sip port : " << mSipPort << std::endl;
	std::cout << "** rcc port : " << mRccAgent.localPort() << std::endl;
	std::cout << std::endl;

	mShutdown = false;
	while (!mShutdown)
	{
		mDum->process(100);

		//check for rcc
		if (mRccAgent.isValid())
			mRccAgent.getAndDispatchMessage(this);

		if (mRegHandle.isValid())
		{
			static const int REG_SPAN_TIME = 3 * 60 * 1000;	//ms
			Timestamp tmNow = Timestamp::NOW();
			if (tmNow - mRegTimestamp > REG_SPAN_TIME)
			{
				mRegTimestamp = tmNow;

				std::cout << tmNow << " : auto registration ......" << std::endl;
				mDum->send(mDum->makeRegistration(NameAddr(mRegURI)));
			}
		}
	}
	// unregister
	unRegisterSession();
	mDum->shutdown(this);
}

bool BasicClientUserAgent::start(const char * sipHost, const char * passwd, unsigned short rccPort, const char * rccIP, unsigned short sipPort)
{
	stop();

	if (!mRccAgent.startAgent(rccPort, rccIP))
		return false;
	
	mSipPort = sipPort;
	mSipHost = sipHost;
	mPassword = passwd;

	setupProfile();

	mDumShutdownRequested = false;

	addTransport(UDP, mSipPort);

	InfoLog(<< "Using local sip port: " << mSipPort);
	// Disable Statistics Manager
	mStack->statisticsManagerEnabled() = false;

	mStack->run();
	mStackThread->run(); 

	this->run();

	return true;
}

void BasicClientUserAgent::stop()
{
   assert(mDum);

   mDumShutdownRequested = true; // Set flag so that shutdown operations can be run in dum process thread

   this->shutdown();
   this->join();

   mStack->shutdownAndJoinThreads();
   mStackThread->shutdown();
   mStackThread->join();

   mRccAgent.stopAgent();
}

/////////////////////////////////

void resip::BasicClientUserAgent::onMessage(RccMessage::MessageType type)
{
}

void resip::BasicClientUserAgent::onMessageAcm(RccMessage::MessageType which, unsigned char result)
{
}

void resip::BasicClientUserAgent::onMessageRgst(const char * callNumber)
{
	registerSession(callNumber);
}

void resip::BasicClientUserAgent::onMessageUrgst(const char * callNumber)
{
	unRegisterSession();
}

void resip::BasicClientUserAgent::onMessageRel(unsigned char reason)
{
	closeSession();
}

void resip::BasicClientUserAgent::onMessageIam(const char * callNumber, RccRtpDataList& rtpDataList)
{
	openSession(callNumber, rtpDataList);
}

void resip::BasicClientUserAgent::onMessageAnm(RccRtpDataList& rtpDataList)
{
	acceptSession(rtpDataList);
}

void resip::BasicClientUserAgent::onMessageTxt(const char * callNumber, const char * txt, unsigned short len)
{
	Data sToURI = makeValidUri(callNumber);

	Data txtData(txt, len);
	auto_ptr<Contents> content(new PlainContents(txtData));

	ClientPagerMessageHandle cpmh = mDum->makePagerMessage(NameAddr(sToURI));
	cpmh.get()->page(content);
}

void resip::BasicClientUserAgent::onInvalidMessage(RccMessage * msg)
{
}

////////////////////////////////////////////////////////

void resip::BasicClientUserAgent::onSuccess(ClientPagerMessageHandle h, const SipMessage & status)
{
	mRccAgent.sendMessageAcm(RccMessage::RCC_TXT);
	h.get()->end();
}

void resip::BasicClientUserAgent::onFailure(ClientPagerMessageHandle h, const SipMessage & status, std::auto_ptr<Contents> contents)
{
	mRccAgent.sendMessageAcm(RccMessage::RCC_TXT, 1);
	h.get()->end();
}

void resip::BasicClientUserAgent::onMessageArrived(ServerPagerMessageHandle sh, const SipMessage & message)
{
	SharedPtr<SipMessage> ok = sh->accept();
	sh->send(ok);

	Data fromUri = message.header(resip::h_From).uri().user();
	Contents *body = message.getContents();

	HeaderFieldValue & hf = body->getHeaderField();
	mRccAgent.sendMessageTxt(fromUri.begin(), body->getHeaderField().getBuffer(), body->getHeaderField().getLength());
}

/////////////////////////////////////

Data resip::BasicClientUserAgent::makeValidUri(const char * uri)
{
	Data sToURI(uri);
	int pos = sToURI.find("sip:");
	if (pos < 0)
		sToURI = Data("sip:") + sToURI;
	pos = sToURI.find("@");
	if (pos < 0)
		sToURI = sToURI + "@" + mSipHost;
	return sToURI;
}

void resip::BasicClientUserAgent::registerSession(const char* num)
{
	mRegURI = Uri(makeValidUri(num));

	NameAddr sipUri = NameAddr(mRegURI);
	// UserProfile Settings
	mProfile->setDefaultFrom(sipUri);
	mProfile->setDigestCredential(mSipHost, mRegURI.user(), mPassword);

	mRegReport = true;
	InfoLog(<< "register for " << sipUri);
	mDum->send(mDum->makeRegistration(sipUri));
}

void resip::BasicClientUserAgent::unRegisterSession()
{
	closeSession();
	// unregister
	if (mRegHandle.isValid())
	{
		mRegHandle->end();
	}

	// end any subscriptions
	if (mServerSubscriptionHandle.isValid())
	{
		mServerSubscriptionHandle->end();
	}
	if (mClientSubscriptionHandle.isValid())
	{
		mClientSubscriptionHandle->end();
	}

}

bool resip::BasicClientUserAgent::openSession(const char * num, RccRtpDataList& rtpDataList)
{
	std::cout << std::endl;
	std::cout << ">>>>> openSession mCallList: " << mCallList.size() << std::endl;
	std::cout << std::endl;

#ifdef ONE_CALL_PERTIME
	if (mCallList.size() > 0)
	{
		mRccAgent.sendMessageAcm(RccMessage::RCC_IAM, 1);
		return false;
	}
#endif

	Data sToURI = makeValidUri(num);
		
//	mCallTarget = Uri(sToURI);
    BasicClientCall* newCall = new BasicClientCall(*this);
	newCall->initiateCall(Uri(sToURI), rtpDataList, mProfile);

	return true;
}

void resip::BasicClientUserAgent::closeSession()
{
	// End all calls - copy list in case delete/unregister of call is immediate
	std::set<BasicClientCall*> tempCallList = mCallList;
	std::set<BasicClientCall*>::iterator it = tempCallList.begin();
	for (; it != tempCallList.end(); it++)
	{
		(*it)->terminateCall();
	}
}

void resip::BasicClientUserAgent::acceptSession(RccRtpDataList& rtpDataList)
{
	std::set<BasicClientCall*>::iterator it = mCallList.begin();
	if (it != mCallList.end())
	{
		(*it)->acceptCall(rtpDataList);
	}
}

void
BasicClientUserAgent::addTransport(TransportType type, int port)
{
	if (mTransport != NULL && mTransport->port() == port)
		return;

	if (mTransport != NULL)
	{
		mStack->removeTransport(mTransport->getKey());
		mTransport = NULL;
	}

	if(port == 0) 
		return;  // Transport disabled

	for (int i=0; i < 10; ++i)
	{
		try
		{
			mTransport = mStack->addTransport(type, port+i);
			mSipPort = port + i;
            return;
		}
		catch (BaseException& e)
		{
			InfoLog (<< "Caught: " << e);
			WarningLog (<< "Failed to add " << Tuple::toData(type) << " transport on " << port);
		}
	}
   throw Transport::Exception("Port already in use", __FILE__, __LINE__);
}

void
BasicClientUserAgent::post(Message* msg)
{
   ConnectionTerminated* terminated = dynamic_cast<ConnectionTerminated*>(msg);
   if (terminated)
   {
      InfoLog(<< "BasicClientUserAgent received connection terminated message for: " << terminated->getFlow());
      delete msg;
      return;
   }
   assert(false);
}

void 
BasicClientUserAgent::onNotifyTimeout(unsigned int timerId)
{
   if(timerId == mCurrentNotifyTimerId)
   {
      sendNotify();
   }
}

void
BasicClientUserAgent::sendNotify()
{
   if(mServerSubscriptionHandle.isValid())
   {
      PlainContents plain("test notify");
      mServerSubscriptionHandle->send(mServerSubscriptionHandle->update(&plain));

      // start timer for next one
      auto_ptr<ApplicationMessage> timer(new NotifyTimer(*this, ++mCurrentNotifyTimerId));
      mStack->post(timer, NotifySendTime, mDum);
   }
}

void 
BasicClientUserAgent::onCallTimeout(BasicClientCall* call)
{
   if(isValidCall(call))
   {
      call->timerExpired();
   }
   /*
   else  // call no longer exists
   {
      // If there are no more calls, then start a new one
      if(mCallList.empty() && !mCallTarget.host().empty())
      {
         // re-start a new call
         BasicClientCall* newCall = new BasicClientCall(*this);
         newCall->initiateCall(mCallTarget, mProfile);
      }
   }*/
}

void 
BasicClientUserAgent::registerCall(BasicClientCall* call)
{
   mCallList.insert(call);
}

void 
BasicClientUserAgent::unregisterCall(BasicClientCall* call)
{
   std::set<BasicClientCall*>::iterator it = mCallList.find(call);
   if(it != mCallList.end())
   {
      mCallList.erase(it);
   }
}

bool 
BasicClientUserAgent::isValidCall(BasicClientCall* call)
{
   std::set<BasicClientCall*>::iterator it = mCallList.find(call);
   if(it != mCallList.end())
   {
      return true;
   }
   return false;
}

void 
BasicClientUserAgent::onDumCanBeDeleted()
{
   mDumShutdown = true;
}

////////////////////////////////////////////////////////////////////////////////
// Registration Handler ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void 
BasicClientUserAgent::onSuccess(ClientRegistrationHandle h, const SipMessage& msg)
{
	InfoLog(<< "onSuccess(ClientRegistrationHandle): msg=" << msg.brief());
	if(mDumShutdownRequested)
	{
		h->end();
		return;
	}
	if(mRegHandle.getId() == 0)  // Note: reg handle id will only be 0 on first successful registration
	{
	}

	mRegHandle = h;
	mRegistrationRetryDelayTime = 0;  // reset

	if (mRegReport)
	{
		mRccAgent.sendMessageAcm(RccMessage::RCC_RGST);
		mRegReport = false;
	}
		
	mRegTimestamp = Timestamp::NOW();
}

void
BasicClientUserAgent::onFailure(ClientRegistrationHandle h, const SipMessage& msg)
{
   InfoLog(<< "onFailure(ClientRegistrationHandle): msg=" << msg.brief());
   mRegHandle = h;
   if(mDumShutdownRequested)
   {
       h->end();
   }

   mRccAgent.sendMessageAcm(RccMessage::RCC_RGST, 1);
}

void
BasicClientUserAgent::onRemoved(ClientRegistrationHandle h, const SipMessage&msg)
{
	InfoLog(<< "onRemoved(ClientRegistrationHandle): msg=" << msg.brief());
	mRegHandle = h;
	mRccAgent.sendMessageAcm(RccMessage::RCC_URGST);
}

int 
BasicClientUserAgent::onRequestRetry(ClientRegistrationHandle h, int retryMinimum, const SipMessage& msg)
{
   mRegHandle = h;
   if(mDumShutdownRequested)
   {
       return -1;
   }

   if(mRegistrationRetryDelayTime == 0)
   {
      mRegistrationRetryDelayTime = BaseRegistrationRetryTimeAllFlowsFailed; // We only have one flow in this test app
   }

   // Use back off procedures of RFC 5626 section 4.5
   mRegistrationRetryDelayTime = resipMin(MaxRegistrationRetryTime, mRegistrationRetryDelayTime * 2);

   // return an evenly distributed random number between 50% and 100% of mRegistrationRetryDelayTime
   int retryTime = Helper::jitterValue(mRegistrationRetryDelayTime, 50, 100);
   InfoLog(<< "onRequestRetry(ClientRegistrationHandle): msg=" << msg.brief() << ", retryTime=" << retryTime);

   mRccAgent.sendMessageAcm(RccMessage::RCC_RGST, 1);
   return retryTime;
}


////////////////////////////////////////////////////////////////////////////////
// InviteSessionHandler ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
BasicClientUserAgent::onNewSession(ClientInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onNewSession(h, oat, msg);
}

void
BasicClientUserAgent::onNewSession(ServerInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
	std::cout << std::endl;
	std::cout << ">>>>> onNewSession(ServerInviteSessionHandle) mCallList: " << mCallList.size() << std::endl;
	std::cout << std::endl;

#ifdef ONE_CALL_PERTIME
	if (mCallList.size() > 1)
	{
		h->reject(486 /* Busy here */);
		dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->terminateCall();
		return;
	}
#endif

   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onNewSession(h, oat, msg);
   /*
   msgIncomingCall(msg.header(resip::h_CallId).value(),
	   msg.header(resip::h_Contacts).front().displayName(),
	   msg.header(resip::h_Contacts).front().uri().user(),
	   msg.header(resip::h_Contacts).front().uri().host(),
	   msg.header(resip::h_RequestLine).uri().host());

   Data frommsg("new call:");*/
   std::cout << std::endl;
   std::cout << "** msg.header(resip::h_From).displayName() : " << msg.header(resip::h_From).displayName() << std::endl;
   std::cout << "** msg.header(resip::h_From).uri().getAor() : " << msg.header(resip::h_From).uri().getAor() << std::endl;
   std::cout << "** msg.header(resip::h_From).uri().user() : " << msg.header(resip::h_From).uri().user() << std::endl;
   std::cout << std::endl;

   mTmpContact  = msg.header(resip::h_From).uri().user();
}

void
BasicClientUserAgent::onFailure(ClientInviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onFailure(h, msg);
}

void
BasicClientUserAgent::onEarlyMedia(ClientInviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onEarlyMedia(h, msg, sdp);
}

void
BasicClientUserAgent::onProvisional(ClientInviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onProvisional(h, msg);
}

void
BasicClientUserAgent::onConnected(ClientInviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onConnected(h, msg);
   mRccAgent.sendMessage(RccMessage::RCC_CONN);
}

void
BasicClientUserAgent::onConnected(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onConnected(h, msg);
   mRccAgent.sendMessage(RccMessage::RCC_CONN);
}

void
BasicClientUserAgent::onStaleCallTimeout(ClientInviteSessionHandle h)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onStaleCallTimeout(h);
}

void
BasicClientUserAgent::onTerminated(InviteSessionHandle h, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onTerminated(h, reason, msg);

   std::cout << std::endl;
   std::cout << ">>>>> onTerminated(InviteSessionHandle) mCallList: " << mCallList.size() << std::endl;
   std::cout << std::endl;

#ifdef ONE_CALL_PERTIME
   if (mCallList.size() > 1)
   {
	   return;
   }
#endif

   mRccAgent.sendMessageRel(reason);

}

void
BasicClientUserAgent::onRedirected(ClientInviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onRedirected(h, msg);
}

///////////////////////////////////////////////////////////////////////////////
///**************************************************************************//

void resip::BasicClientUserAgent::getRemoteOffer(const SdpContents & sdp, RccRtpDataList& rtpDataList)
{
	Data rtpip = sdp.session().origin().getAddress();
	for (list<SdpContents::Session::Medium>::const_iterator i = sdp.session().media().begin();
		i != sdp.session().media().end(); ++i)
	{
		if (i->name() == Symbols::audio || i->name() == "video")
		{
			RccRtpData data(sdp.session().origin().getAddress().begin(), i->port(),
				i->codecs().begin()->payloadType(), i->codecs().begin()->getRate());
			rtpDataList.push_back(data);
		}
	}
}

void
BasicClientUserAgent::onAnswer(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
	BasicClientCall *call = dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get());
	if (call != NULL)
	{
		call->onAnswer(h, msg, sdp);
		
		RccRtpDataList rtpDataList;
		getRemoteOffer(sdp, rtpDataList);
		mRccAgent.sendMessageAnm(rtpDataList);
	}
}

void
BasicClientUserAgent::onOffer(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{         
	BasicClientCall *call = dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get());
	if (call != NULL)
	{
		call->onOffer(h, msg, sdp);
		
		RccRtpDataList rtpDataList;
		getRemoteOffer(sdp, rtpDataList);
		mRccAgent.sendMessageIam(mTmpContact.begin(), rtpDataList);
	}
}

void
BasicClientUserAgent::onOfferRequired(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onOfferRequired(h, msg);
}

void
BasicClientUserAgent::onOfferRejected(InviteSessionHandle h, const SipMessage* msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onOfferRejected(h, msg);
}

void
BasicClientUserAgent::onOfferRequestRejected(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onOfferRequestRejected(h, msg);
}

void
BasicClientUserAgent::onRemoteSdpChanged(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onRemoteSdpChanged(h, msg, sdp);
}

void
BasicClientUserAgent::onInfo(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onInfo(h, msg);
}

void
BasicClientUserAgent::onInfoSuccess(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onInfoSuccess(h, msg);
}

void
BasicClientUserAgent::onInfoFailure(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onInfoFailure(h, msg);
}

void
BasicClientUserAgent::onRefer(InviteSessionHandle h, ServerSubscriptionHandle ssh, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onRefer(h, ssh, msg);
}

void
BasicClientUserAgent::onReferAccepted(InviteSessionHandle h, ClientSubscriptionHandle csh, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onReferAccepted(h, csh, msg);
}

void
BasicClientUserAgent::onReferRejected(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onReferRejected(h, msg);
}

void
BasicClientUserAgent::onReferNoSub(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onReferNoSub(h, msg);
}

void
BasicClientUserAgent::onMessage(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onMessage(h, msg);
}

void
BasicClientUserAgent::onMessageSuccess(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onMessageSuccess(h, msg);
}

void
BasicClientUserAgent::onMessageFailure(InviteSessionHandle h, const SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onMessageFailure(h, msg);
}

void
BasicClientUserAgent::onForkDestroyed(ClientInviteSessionHandle h)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onForkDestroyed(h);
}

void 
BasicClientUserAgent::onReadyToSend(InviteSessionHandle h, SipMessage& msg)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onReadyToSend(h, msg);
}

void 
BasicClientUserAgent::onFlowTerminated(InviteSessionHandle h)
{
   dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get())->onFlowTerminated(h);
}


////////////////////////////////////////////////////////////////////////////////
// DialogSetHandler ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void 
BasicClientUserAgent::onTrying(AppDialogSetHandle h, const SipMessage& msg)
{
   BasicClientCall *call = dynamic_cast<BasicClientCall *>(h.get());
   if(call)
   {
      call->onTrying(h, msg);
	  mRccAgent.sendMessageAcm(RccMessage::RCC_IAM);
   }
   else
   {
      InfoLog(<< "onTrying(AppDialogSetHandle): " << msg.brief());
   }

}

void 
BasicClientUserAgent::onNonDialogCreatingProvisional(AppDialogSetHandle h, const SipMessage& msg)
{
   BasicClientCall *call = dynamic_cast<BasicClientCall *>(h.get());
   if(call)
   {
      call->onNonDialogCreatingProvisional(h, msg);
   }
   else
   {
      InfoLog(<< "onNonDialogCreatingProvisional(AppDialogSetHandle): " << msg.brief());
   }
}

////////////////////////////////////////////////////////////////////////////////
// ClientSubscriptionHandler ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
BasicClientUserAgent::onUpdatePending(ClientSubscriptionHandle h, const SipMessage& msg, bool outOfOrder)
{
   BasicClientCall* call = dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get());
   if(call)
   {
      call->onUpdatePending(h, msg, outOfOrder);
      return;
   }
   InfoLog(<< "onUpdatePending(ClientSubscriptionHandle): " << msg.brief());
   h->acceptUpdate();
}

void
BasicClientUserAgent::onUpdateActive(ClientSubscriptionHandle h, const SipMessage& msg, bool outOfOrder)
{
   BasicClientCall* call = dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get());
   if(call)
   {
      call->onUpdateActive(h, msg, outOfOrder);
      return;
   }
   InfoLog(<< "onUpdateActive(ClientSubscriptionHandle): " << msg.brief());
   h->acceptUpdate();
}

void
BasicClientUserAgent::onUpdateExtension(ClientSubscriptionHandle h, const SipMessage& msg, bool outOfOrder)
{
   BasicClientCall* call = dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get());
   if(call)
   {
      call->onUpdateExtension(h, msg, outOfOrder);
      return;
   }
   InfoLog(<< "onUpdateExtension(ClientSubscriptionHandle): " << msg.brief());
   h->acceptUpdate();
}

void 
BasicClientUserAgent::onNotifyNotReceived(ClientSubscriptionHandle h)
{
   BasicClientCall* call = dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get());
   if(call)
   {
      call->onNotifyNotReceived(h);
      return;
   }
   WarningLog(<< "onNotifyNotReceived(ClientSubscriptionHandle)");
   h->end();
}

void
BasicClientUserAgent::onTerminated(ClientSubscriptionHandle h, const SipMessage* msg)
{
   BasicClientCall* call = dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get());
   if(call)
   {
      call->onTerminated(h, msg);
      return;
   }
   if(msg)
   {
      InfoLog(<< "onTerminated(ClientSubscriptionHandle): msg=" << msg->brief());
   }
   else
   {
      InfoLog(<< "onTerminated(ClientSubscriptionHandle)");
   }
   mRccAgent.sendMessageRel();
}

void
BasicClientUserAgent::onNewSubscription(ClientSubscriptionHandle h, const SipMessage& msg)
{
   BasicClientCall* call = dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get());
   if(call)
   {
      call->onNewSubscription(h, msg);
      return;
   }
   mClientSubscriptionHandle = h;
   InfoLog(<< "onNewSubscription(ClientSubscriptionHandle): msg=" << msg.brief());
}

int 
BasicClientUserAgent::onRequestRetry(ClientSubscriptionHandle h, int retrySeconds, const SipMessage& msg)
{
   BasicClientCall* call = dynamic_cast<BasicClientCall *>(h->getAppDialogSet().get());
   if(call)
   {
      return call->onRequestRetry(h, retrySeconds, msg);
   }
   InfoLog(<< "onRequestRetry(ClientSubscriptionHandle): msg=" << msg.brief());
   return FailedSubscriptionRetryTime;  
}

////////////////////////////////////////////////////////////////////////////////
// ServerSubscriptionHandler ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void 
BasicClientUserAgent::onNewSubscription(ServerSubscriptionHandle h, const SipMessage& msg)
{
   InfoLog(<< "onNewSubscription(ServerSubscriptionHandle): " << msg.brief());

   mServerSubscriptionHandle = h;
   mServerSubscriptionHandle->setSubscriptionState(Active);
   mServerSubscriptionHandle->send(mServerSubscriptionHandle->accept());
   sendNotify();
}

void 
BasicClientUserAgent::onNewSubscriptionFromRefer(ServerSubscriptionHandle ss, const SipMessage& msg)
{
   InfoLog(<< "onNewSubscriptionFromRefer(ServerSubscriptionHandle): " << msg.brief());
   // Received an out-of-dialog refer request with implicit subscription
   try
   {
      if(msg.exists(h_ReferTo))
      {
         // Check if TargetDialog header is present
         if(msg.exists(h_TargetDialog))
         {
            pair<InviteSessionHandle, int> presult;
            presult = mDum->findInviteSession(msg.header(h_TargetDialog));
            if(!(presult.first == InviteSessionHandle::NotValid())) 
            {         
               BasicClientCall* callToRefer = (BasicClientCall*)presult.first->getAppDialogSet().get();

               callToRefer->onRefer(presult.first, ss, msg);
               return;
            }
         }

         // We don't support ood refers that don't target a dialog - reject request 
         WarningLog (<< "onNewSubscriptionFromRefer(ServerSubscriptionHandle): Received ood refer (noSub) w/out a Target-Dialog: " << msg.brief());
         ss->send(ss->reject(400));
      }
      else
      {
         WarningLog (<< "onNewSubscriptionFromRefer(ServerSubscriptionHandle): Received refer w/out a Refer-To: " << msg.brief());
         ss->send(ss->reject(400));
      }
   }
   catch(BaseException &e)
   {
      WarningLog(<< "onNewSubscriptionFromRefer(ServerSubscriptionHandle): exception " << e);
   }
   catch(...)
   {
      WarningLog(<< "onNewSubscriptionFromRefer(ServerSubscriptionHandle): unknown exception");
   }
}

void 
BasicClientUserAgent::onRefresh(ServerSubscriptionHandle, const SipMessage& msg)
{
   InfoLog(<< "onRefresh(ServerSubscriptionHandle): " << msg.brief());
}

void 
BasicClientUserAgent::onTerminated(ServerSubscriptionHandle)
{
   InfoLog(<< "onTerminated(ServerSubscriptionHandle)");
}

void 
BasicClientUserAgent::onReadyToSend(ServerSubscriptionHandle, SipMessage&)
{
}

void 
BasicClientUserAgent::onNotifyRejected(ServerSubscriptionHandle, const SipMessage& msg)
{
   WarningLog(<< "onNotifyRejected(ServerSubscriptionHandle): " << msg.brief());
}

void 
BasicClientUserAgent::onError(ServerSubscriptionHandle, const SipMessage& msg)
{
   WarningLog(<< "onError(ServerSubscriptionHandle): " << msg.brief());
}

void 
BasicClientUserAgent::onExpiredByClient(ServerSubscriptionHandle, const SipMessage& sub, SipMessage& notify)
{
   InfoLog(<< "onExpiredByClient(ServerSubscriptionHandle): " << notify.brief());
}

void 
BasicClientUserAgent::onExpired(ServerSubscriptionHandle, SipMessage& msg)
{
   InfoLog(<< "onExpired(ServerSubscriptionHandle): " << msg.brief());
}

bool 
BasicClientUserAgent::hasDefaultExpires() const
{
   return true;
}

UInt32 
BasicClientUserAgent::getDefaultExpires() const
{
   return 60;
}

////////////////////////////////////////////////////////////////////////////////
// OutOfDialogHandler //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void 
BasicClientUserAgent::onSuccess(ClientOutOfDialogReqHandle, const SipMessage& msg)
{
   InfoLog(<< "onSuccess(ClientOutOfDialogReqHandle): " << msg.brief());
}

void 
BasicClientUserAgent::onFailure(ClientOutOfDialogReqHandle h, const SipMessage& msg)
{
   WarningLog(<< "onFailure(ClientOutOfDialogReqHandle): " << msg.brief());
}

void 
BasicClientUserAgent::onReceivedRequest(ServerOutOfDialogReqHandle ood, const SipMessage& msg)
{
   InfoLog(<< "onReceivedRequest(ServerOutOfDialogReqHandle): " << msg.brief());

   switch(msg.method())
   {
   case OPTIONS:
      {
         SharedPtr<SipMessage> optionsAnswer = ood->answerOptions();
         ood->send(optionsAnswer);
         break;
      }
   default:
      ood->send(ood->reject(501 /* Not Implemented*/));
      break;
   }
}

////////////////////////////////////////////////////////////////////////////////
// RedirectHandler /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void 
BasicClientUserAgent::onRedirectReceived(AppDialogSetHandle h, const SipMessage& msg)
{
   BasicClientCall* call = dynamic_cast<BasicClientCall *>(h.get());
   if(call)
   {
      call->onRedirectReceived(h, msg);
   }
   else
   {
      InfoLog(<< "onRedirectReceived(AppDialogSetHandle): " << msg.brief());
   }
}

bool 
BasicClientUserAgent::onTryingNextTarget(AppDialogSetHandle, const SipMessage& msg)
{
   InfoLog(<< "onTryingNextTarget(AppDialogSetHandle): " << msg.brief());

   // Always allow redirection for now
   return true;
}




/* ====================================================================

 Copyright (c) 2011, SIP Spectrum, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are 
 met:

 1. Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer. 

 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution. 

 3. Neither the name of SIP Spectrum nor the names of its contributors 
    may be used to endorse or promote products derived from this 
    software without specific prior written permission. 

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ==================================================================== */

