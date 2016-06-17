//#include <vld.h>
#include <iostream>
#include <string>
#include "SipUserAgent.hxx"

using namespace std;

class MySipUserAgent : public SipUserAgent
{
public:
	virtual void OnMsgRegister(std::string sMsg) {cout<<sMsg<<endl;};
	virtual void OnMsgNOTIFY(std::string sMsg) {cout<<sMsg<<endl;};

	virtual void OnIncomingCall(std::string sCallId, std::string sDisplayName, std::string sUserName, std::string sFromURI, std::string sToURI) {id = sCallId;};

public:
	string id;
};

int main(int argc, char *argv[]) 
{

	MySipUserAgent *dialer=new MySipUserAgent();

/*	char buf[128];
	int len=GetCurrentDirectoryA(128,buf);
	if(len>0)
	{
		string data=buf;
		data+="/toy.wav";
//		dialer->setPlayFile(string(data.c_str()));
	}*/

	dialer->Initialize(true,
		"",
		0,
		string("bobo"),/*displayname*/
		string(""), /*outboundproxy*/
		string("192.168.1.18"),//sip99.konceptusa.com")/*std::string sSIPProxy*/,
		string("444")/*std::string sLoginId*/, 
		string("444")/*std::string sLoginPwd*/, 
		true/*bool bUseSoundDevice*/, 
		8/*int nTotalLine);*/);

	dialer->SetLogType("cout","INFO"/*"STACK"*/);
	dialer->SelectAllVoiceCodec();

	dialer->RegisterToProxy(1000);

	dialer->OpenLine(1,false,"",8000);
	dialer->OpenLine(2,false,"",8010);

   while(1)
	{
		string command;
		string value;

		cin>>command;
		if(command==string("call"))
		{
			cin>>value;
			dialer->Connect(1,value,0,1);
		}
		else if(command==string("answer"))
		{
			dialer->AcceptCall(2,(dialer->id).c_str(),0,0);
		}
		else if(command==string("hangup"))
		{
			dialer->Disconnect(1);
		}
		else if(command==string("reject"))
		{
			dialer->RejectCall((dialer->id).data());
		}
		else if(command==string("hold"))
		{
			dialer->HoldLine(1);
		}
		else if(command==string("unhold"))
		{
			dialer->UnHoldLine(1);
		}
		else if(command==string("quit"))
		{
			break;
		}
		else if(command==string("setval"))
		{
			cin>>value;
			dialer->SetSpkVolume( atoi(value.c_str()));
		}
		else if(command==string("getval"))
		{
			int val=dialer->GetSpkVolume();
			cout<<"valume="<<val<<endl;
		}
		else if(command==string("regis"))
		{
			dialer->RegisterToProxy(200);
		}
		else if(command==string("unreg"))
		{
			dialer->UnRegisterToProxy();
		}
		else if(command==string("vcode"))
		{
			dialer->SelectVideoCodec(98);
		}
		else if(command==string("dt"))
		{
			dialer->DigitDTMF(1, string("1").c_str());

			dialer->DigitDTMF(1, string("2").c_str());

			dialer->DigitDTMF(1, string("3").c_str());
		}
		
		else if(command==string(""))
		{
		}
	}
	delete dialer;

	return 0;
}

