
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/AppDialog.hxx"
#include "resip/dum/AppDialogSet.hxx"
#include "resip/dum/AppDialogSetFactory.hxx"
#include "AppSession.hxx"

/////////////////////////////////////////////////////////////////////////////////
//
// Classes that provide the mapping between Application Data and DUM 
// dialogs/dialogsets
//  										
// The DUM layer creates an AppDialog/AppDialogSet object for inbound/outbound
// SIP Request that results in Dialog creation.
//  										
/////////////////////////////////////////////////////////////////////////////////
class MyAppDialog : public resip::AppDialog
{
public:
   MyAppDialog(resip::HandleManager& ham);
   MyAppDialog(resip::HandleManager& ham, AppSession *session);

  virtual ~MyAppDialog();
  AppSession *getCallSession();
  void setCallSession(AppSession *session); 

protected:
	AppSession *mSession;
};

class MyAppDialogSet : public AppDialogSet
{
public:
  MyAppDialogSet(resip::DialogUsageManager& dum);
  MyAppDialogSet(resip::DialogUsageManager& dum, AppSession *session, resip::SharedPtr<resip::UserProfile>& userProfile);
  virtual ~MyAppDialogSet();
  virtual resip::AppDialog* createAppDialog(const resip::SipMessage& msg);
  // virtual resip::SharedPtr<resip::UserProfile> getUserProfile();
  virtual resip::SharedPtr<resip::UserProfile> selectUASUserProfile(const resip::SipMessage& msg);
  void setCallSession(AppSession *session)
    { this->mSession = session; };

  
protected:
  AppSession *mSession;
  resip::SharedPtr<resip::UserProfile> userProfile;

};

class MyAppDialogSetFactory : public AppDialogSetFactory
{
public:
   virtual AppDialogSet* createAppDialogSet(DialogUsageManager& dum, const SipMessage& msg);

};

