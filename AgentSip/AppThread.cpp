
#include "AppThread.hxx"
#include "TaskManager.hxx"
#include "DumRecurringTask.hxx"
#include "DialInstance.hxx"

#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace resip;

AppThread::AppThread(DialInstance* task,DumRecurringTask* dumtask): 
callManager(task),
dumRecure(dumtask)
{
	taskManager = new TaskManager();

	taskManager->addRecurringTask(dumRecure);
	taskManager->addRecurringTask(callManager);
}

AppThread::~AppThread()
{
   delete taskManager;
}

void
AppThread::thread()
{
	taskManager->start();
}

void AppThread::logStats()
{
//  callManager->logStats();
}

void AppThread::shutdown()
{
  //B2BUA_LOG_CRIT("B2BUA::stop not implemented!");
  //assert(0);
  taskManager->stop();
  ThreadIf::shutdown();
}
