
#ifndef PHONELINE_H
#define PHONELINE_H

#include <string>
#include <vector>
#include "rutil/Data.hxx"

class AppSession;

class PhoneLine
{
public:
	PhoneLine(int no);
	~PhoneLine();

	void setRTPRxIP(resip::Data ip) {mRTPRxIP=ip;};
	resip::Data getRTPRxIP() {return mRTPRxIP;};

	void setRTPRxPort(int port) {mRTPRxPort=port;};
	int getRTPRxPort() {return mRTPRxPort;};
	int getRTPRxVideoPort() {return mRTPRxVideoPort;};

	int getLineNo() const {return mLineNo;};

	void setHoldState(bool hold) ;
	bool getHoldState();

	void setLineState(bool line);
	bool getLineState() const;

	void AddSession( AppSession *sess);
	void RemSession(AppSession *sess);

private:

	int mLineNo;
	resip::Data mRTPRxIP;
	int mRTPRxPort;
	int mRTPRxVideoPort;

	bool mLineOpen;
	bool mLineHold;
	bool mLineBusy;
	std::vector<AppSession*> mSessionList;
};

#endif
