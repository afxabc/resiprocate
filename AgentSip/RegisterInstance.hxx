
#ifndef __RESGISTERINSTANCE_H
#define __RESGISTERINSTANCE_H

#include "resip/dum/DialogUsageManager.hxx"
#include "resip/stack/Uri.hxx"
#include "rutil/Data.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/stack/SipStack.hxx"
#include "resip/dum/RegistrationHandler.hxx"

#include "DialerConfiguration.hxx"

class MyRegistrationHandler;
class MySipDialer;

class RegisterInstance {

public:
   RegisterInstance(DialerConfiguration& dialerConfiguration);
   ~RegisterInstance();
   
   typedef enum 
   {
      RegSuccessful,
      RegUnsuccessful,
	  UnRegSuccessful,
      Error
   } RegisterResult;
   RegisterResult execute();

   void setSipStack(resip::SipStack *stack);
   void setDum(resip::DialogUsageManager *dum);

   void sendRegister(int nExpire);
//   resip::Data processNumber(const resip::Data& verboseNumber);

   // Receive notifications from MyInviteSessionHandler

   // some kind of failure message (4xx, 5xx, 6xx) received
   void onFailure(resip::ClientRegistrationHandle crh, const resip::SipMessage& response);
   // the session has connected and is ready for a REFER
   void onSuccess(resip::ClientRegistrationHandle crh, const resip::SipMessage& response);
   // the REFER succeeded
   void onRemoved(resip::ClientRegistrationHandle crh, const resip::SipMessage& response);

   void msgTryingToRegister();
	void msgFailToRegister();
	void msgSuccessToRegister();

	void msgTryingToReRegister();
	void msgFailToReRegister();
	void msgSuccessToReRegister();
	
	void msgTryingToUnRegister();
	void msgFailToUnRegister();
	void msgSuccessToUnRegister();

   MySipDialer *mSipDialer;

private:
   // Copy of values supplied when instance created
   DialerConfiguration &mDialerConfiguration;
   resip::Uri mTargetUri;
   // The target URI, converted to a sip: URI if necessary
   resip::Uri mFullTarget;
   resip::ClientRegistration* mCR;

   resip::SipStack *mSipStack;
   resip::DialogUsageManager *mDum;

   MyRegistrationHandler *regHandler;
   
   // MyInviteSessionHandler will notify us of progress
   friend class MyRegistrationHandler;

   RegisterResult mResult;
   

};

#endif

