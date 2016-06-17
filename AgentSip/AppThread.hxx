
#ifndef AppThread__hxx
#define AppThread__hxx

#include "rutil/ThreadIf.hxx"
#include "TaskManager.hxx"

class DialInstance;
class DumRecurringTask;

/** 
   @ingroup resip_crit
   @brief This class is used to create a thread to run the SipStack in.

   The thread provides cycles to the SipStack by calling process.  Process
   is called at least every 25ms, or sooner if select returns a signaled
   file descriptor.  This is also the canonical example code for how to run
   SipStack's process-loop, if you want to do it yourself.
*/
class AppThread : public resip::ThreadIf
{
   public:
      AppThread(DialInstance* task,DumRecurringTask* dumtask);
      virtual ~AppThread();
      
      virtual void thread();

protected:

  TaskManager *taskManager;
  DialInstance *callManager;
  DumRecurringTask *dumRecure;

public:
  // Send some stats about the system to syslog
  virtual void logStats();

  // Indicate to the B2BUA that it should shutdown cleanly
  // (stop all calls, wait for all managers to stop)
  virtual void shutdown();

};



#endif
