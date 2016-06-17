

//#include "resip/dum/ClientAuthManager.hxx"
//#include "resip/dum/DialogUsageManager.hxx"
//#include "resip/dum/MasterProfile.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/stack/SipStack.hxx"
#include "resip/stack/Uri.hxx"
#include "rutil/Data.hxx"
#include "rutil/SharedPtr.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/ClientRegistration.hxx"

#include "DialerConfiguration.hxx"
#include "RegisterInstance.hxx"
#include "MyRegistrationHandler.hxx"
#include "MySipDialer.hxx"

using namespace resip;
using namespace std;

RegisterInstance::RegisterInstance(DialerConfiguration& dialerConfiguration) :
   mDialerConfiguration(dialerConfiguration),
   mResult(Error),
   regHandler(NULL),
   mCR(NULL)
{
}

RegisterInstance::~RegisterInstance()
{
	if(regHandler)
	{
		delete regHandler;
	}
}

RegisterInstance::RegisterResult RegisterInstance::execute()
{
	int expire=mDialerConfiguration.getSendRegister();
	if(expire==0)
	{
		expire=100;
	}
	mDum->getMasterProfile()->setDefaultRegistrationTime(expire);

	regHandler=new MyRegistrationHandler(*this);

	mDum->setClientRegistrationHandler(regHandler);
	mDum->getMasterProfile()->setDigestCredential(mDialerConfiguration.getRegIdentity().host(),
		mDialerConfiguration.getRegIdentity().user(),mDialerConfiguration.getAuthPassword());
	SharedPtr<SipMessage> regMessage = mDum->makeRegistration(NameAddr(mDialerConfiguration.getRegIdentity()));
	mDum->send(regMessage);

	msgTryingToRegister();

	return mResult;
}

void RegisterInstance::sendRegister(int nExpire) 
{
	if(nExpire<0)
	{
		if(mCR)
		{
			mCR->end();
		}
	}
	else
	{
		if(nExpire==0)
		{
			nExpire=100;
		}
		SharedPtr<UserProfile> outboundUserProfile(mDum->getMasterUserProfile());
		outboundUserProfile->setDefaultFrom(mDialerConfiguration.getDialerIdentity());
		outboundUserProfile->setDefaultRegistrationTime(nExpire);

		outboundUserProfile->setDigestCredential(mDialerConfiguration.getAuthRealm(), 
			mDialerConfiguration.getAuthUser(), 
			mDialerConfiguration.getAuthPassword());
		SharedPtr<SipMessage> regMessage = mDum->makeRegistration(NameAddr(mDialerConfiguration.getRegIdentity()));

		mDum->send(regMessage);
		msgTryingToRegister();
	}

}

void
RegisterInstance::setSipStack(SipStack *stack)
{
	mSipStack=stack;
}

void
RegisterInstance::setDum(DialogUsageManager *dum)
{
	mDum=dum;
}

void RegisterInstance::onFailure(ClientRegistrationHandle crh, const SipMessage& response)
{
	mCR=crh.get();
   mResult = RegUnsuccessful;

   msgFailToRegister();
}

void RegisterInstance::onSuccess(ClientRegistrationHandle crh, const SipMessage& response) 
{
	mCR=crh.get();
   mResult = RegSuccessful;

   msgSuccessToRegister();
}

void RegisterInstance::onRemoved(ClientRegistrationHandle crh, const SipMessage& response) 
{
	mResult = UnRegSuccessful;

   msgSuccessToUnRegister();
}

void RegisterInstance::msgTryingToRegister()
{
	mSipDialer->OnTryingToRegister();
}

void RegisterInstance::msgFailToRegister()
{
	mSipDialer->OnFailToRegister();
}

void RegisterInstance::msgSuccessToRegister()
{
	mSipDialer->OnSuccessToRegister();
}

void RegisterInstance::msgTryingToReRegister()
{
	mSipDialer->OnTryingToReRegister();
}

void RegisterInstance::msgFailToReRegister()
{
	mSipDialer->OnFailToReRegister();
}

void RegisterInstance::msgSuccessToReRegister()
{
	mSipDialer->OnSuccessToReRegister();
}

void RegisterInstance::msgTryingToUnRegister()
{
	mSipDialer->OnTryingToUnRegister();
}

void RegisterInstance::msgFailToUnRegister()
{
	mSipDialer->OnFailToUnRegister();
}

void RegisterInstance::msgSuccessToUnRegister()
{
	mSipDialer->OnSuccessToUnRegister();
}



