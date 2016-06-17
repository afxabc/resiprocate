
#include "PhoneLine.hxx"
#include "DialInstance.hxx"
#include "AppSession.hxx"


using namespace resip;

PhoneLine::PhoneLine(int no):
mLineNo(no),
mRTPRxPort(7078+no*20),
mLineOpen(true),
mLineHold(false),
mLineBusy(false)
{
	mRTPRxVideoPort = mRTPRxPort+2000;
}

PhoneLine::~PhoneLine()
{
}

void PhoneLine::setHoldState(bool hold)
{
	if( mSessionList.size() > 1 )
	{
		return;
	}
	else
	{
		mLineHold=hold;
	}
}

bool PhoneLine::getHoldState()
{
	return mLineHold;
}

void PhoneLine::setLineState(bool line)
{
	if( mSessionList.size() > 1 )
	{
		return;
	}
	else
	{
		mLineBusy=line;
	}
}

bool PhoneLine::getLineState() const
{
	return mLineBusy;
}

void PhoneLine::AddSession( AppSession *sess)
{
	mSessionList.push_back(sess);
}

void PhoneLine::RemSession(AppSession *sess)
{
	std::vector<AppSession*>::iterator i;
	for(i = mSessionList.begin(); i != mSessionList.end(); ++i)
	{
		if( (*i) == sess )
		{
			mSessionList.erase(i);
			break;
		}
	}
}


