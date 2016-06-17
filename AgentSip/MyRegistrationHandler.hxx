
#ifndef __MyRegistrationHandler_h
#define __MyRegistrationHandler_h

#include "resip/stack/SipMessage.hxx"
#include "resip/dum/RegistrationHandler.hxx"
#include "RegisterInstance.hxx"
//#include "resip/dum/Handles.hxx"

using namespace resip;

class MyRegistrationHandler : public resip::ClientRegistrationHandler {

protected:
  RegisterInstance& mRegInstance;

public:
	  MyRegistrationHandler(RegisterInstance &instance);
      virtual ~MyRegistrationHandler();
      /// Called when registraion succeeds or each time it is sucessfully
      /// refreshed. 
      virtual void onSuccess(ClientRegistrationHandle, const SipMessage& response);

      // Called when all of my bindings have been removed
      virtual void onRemoved(ClientRegistrationHandle, const SipMessage& response);
      
      /// call on Retry-After failure. 
      /// return values: -1 = fail, 0 = retry immediately, N = retry in N seconds
      virtual int onRequestRetry(ClientRegistrationHandle, int retrySeconds, const SipMessage& response);
      
      /// Called if registration fails, usage will be destroyed (unless a 
      /// Registration retry interval is enabled in the Profile)
      virtual void onFailure(ClientRegistrationHandle, const SipMessage& response);

};


#endif
