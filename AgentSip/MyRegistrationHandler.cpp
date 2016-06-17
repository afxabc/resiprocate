

#include "rutil/Data.hxx"
#include "MyRegistrationHandler.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

MyRegistrationHandler::MyRegistrationHandler(RegisterInstance &instance):
	mRegInstance(instance)
{
}

MyRegistrationHandler::~MyRegistrationHandler()
{
}

void MyRegistrationHandler::onSuccess(ClientRegistrationHandle crh, const SipMessage& response)
{
	InfoLog(<<"MyRegistrationHandler::onSuccess");
	mRegInstance.onSuccess(crh, response);
}

void MyRegistrationHandler::onRemoved(ClientRegistrationHandle crh, const SipMessage& response)
{
	InfoLog(<<"MyRegistrationHandler::onRemoved");
	mRegInstance.onRemoved(crh, response);
}

int MyRegistrationHandler::onRequestRetry(ClientRegistrationHandle crh, int retrySeconds, const SipMessage& response)
{
	InfoLog(<<"MyRegistrationHandler::onRemoved");
	return 0;
}

void MyRegistrationHandler::onFailure(ClientRegistrationHandle crh, const SipMessage& response)
{
	InfoLog(<<"MyRegistrationHandler::onRemoved");
	mRegInstance.onFailure(crh, response);
}
