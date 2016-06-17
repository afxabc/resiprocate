
#include <iostream>
#include "MyAppDialogSet.hxx"

#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP


MyAppDialog::MyAppDialog(HandleManager& ham) : AppDialog(ham) 
{
  setCallSession(NULL);
  InfoLog(<<  ": MyAppDialog: created.");
}

MyAppDialog::MyAppDialog(HandleManager& ham, AppSession *session) : AppDialog(ham)
{
  setCallSession(session);
  // FIXME - is this the B Leg?
  if(mSession != NULL)
//    mSession->setBLegAppDialog(this);

  InfoLog(<<  ": MyAppDialog: created.");
}

MyAppDialog::~MyAppDialog() 
{
  if(mSession != NULL)
    mSession->releaseAppDialog(this);

  InfoLog(<<  ": MyAppDialog: delete.");
}

AppSession *MyAppDialog::getCallSession() 
{
  return mSession;
}

void MyAppDialog::setCallSession(AppSession *session) 
{
  this->mSession = session;
}

MyAppDialogSet::MyAppDialogSet(DialogUsageManager& dum) : AppDialogSet(dum)
{
  this->mSession = NULL;
  //userProfile = NULL;
  InfoLog(<<  ": MyAppDialogSet: created.");
}

MyAppDialogSet::MyAppDialogSet(DialogUsageManager& dum, AppSession *session, SharedPtr<UserProfile>& userProfile) : AppDialogSet(dum) 
{
  this->mSession = session;
  this->userProfile = userProfile;

  InfoLog(<<  ": MyAppDialogSet: created.");
}

MyAppDialogSet::~MyAppDialogSet() 
{
  if(mSession != NULL)
    mSession->releaseAppDialogSet(this);

  InfoLog(<<  ": MyAppDialogSet: delete.");
}

AppDialog* MyAppDialogSet::createAppDialog(const SipMessage& msg) 
{
  return new MyAppDialog(mDum, mSession);
}

/* SharedPtr<UserProfile> MyAppDialogSet::getUserProfile() {
  return userProfile;
} */

SharedPtr<UserProfile> MyAppDialogSet::selectUASUserProfile(const SipMessage& msg)
{
  //return getUserProfile();
  return mDum.getMasterUserProfile();
}

AppDialogSet* MyAppDialogSetFactory::createAppDialogSet(DialogUsageManager& dum, const SipMessage& msg)
{
  return new MyAppDialogSet(dum);
}


