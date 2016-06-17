/********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: msticker
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\msticker.cpp
*********************************************************************/
#include <cassert>
#include <iostream>
#include "msticker.h"
#include "rutil/Lock.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

//----------------------------------------------------------------------------
// msticker.cpp                                                                  
//----------------------------------------------------------------------------
using namespace std;
//## package Default 

//## class msticker 
unsigned __int64 MSTicker::get_cur_time()
{
#if defined(WIN32)
	return timeGetTime() ;
#else
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME,&ts)<0){
		ms_fatal("clock_gettime() doesn't work: %s",strerror(errno));
	}
	return (ts.tv_sec*1000LL) + (ts.tv_nsec/1000000LL);
#endif
}

MSTicker::MSTicker(int val)
{
	interval = val;
	ticks = 1;
	time = 0L;
	running = false;
#ifdef WIN32_TIMERS
	TimeEvent = NULL;
#endif
#ifdef TEST_VERSION
	expired = 0;
#endif
}

MSTicker::~MSTicker() 
{
}

int MSTicker::getinterval() const
{
	return interval;
}

void MSTicker::setinterval(int val)
{
	interval = val;
}

void MSTicker::listExecute()
{
	executechain.process(context);
}

void MSTicker::restart()
{
	if(!running)
	{
		if(this->isShutdown())
			mShutdown = false;
		
		run();
	}
#ifdef TEST_VERSION
	expired = 0;
#endif
}
#ifndef WIN32_TIMERS

void MSTicker::thread()
{
	unsigned __int64 realtime;
	__int64 diff;
	int lastlate=0;
	int precision=2;
	int late;
	
	Lock* lock=new Lock(mutex);
	running=true;
	orig = get_cur_time();
	time = 0;
	
    while(!isShutdown())
    {
		precision = set_high_prio();

		 listExecute();

		 time += interval;
		 unset_high_prio(precision);
		 delete lock;

		 while(1)
		 {
			 realtime = get_cur_time() - orig;
			 
			 diff = time - realtime;
			 if (diff > 0)
			 {
				 /* sleep until next tick */
				 sleepMs(diff);
			 }
			 else
			 {
				 late=-diff;
				 if (late>interval*5)
				 {
					 InfoLog(<<"We are late of "<<late<<" miliseconds.");
				 }
				 break; /*exit the while loop */
			 }
//			 lock=new Lock(mutex);
		 }
		 lock=new Lock(mutex);

#ifdef TEST_VERSION
		 if( expired > 9000 )
		 {
			 assert(0);
		 }
		 expired++;
#endif
	}
	delete lock;
	running=false;
	
	InfoLog(<<"\nMSTicker thread exiting\n");
}


#else

void MSTicker::thread()
{
	unsigned __int64 realtime;
	int precision=2;
	UINT timerId;

	precision = set_high_prio();

	TimeEvent = CreateEvent(NULL, false, false, NULL);

	ticks=1;
//	ms_mutex_lock(&s->lock);
	orig=get_cur_time();

	timerId = timeSetEvent (interval, precision, (LPTIMECALLBACK)TimeEvent, 0, TIME_CALLBACK_EVENT_SET);
	while(!isShutdown())
	{
		DWORD err;

		ticks++;
		listExecute();

		/* elapsed time since origin */
		time = get_cur_time()- orig;

//		ms_mutex_unlock(&s->lock);
		err = WaitForSingleObject (TimeEvent, interval*20 ); /* wake up each diff */
		if (err==WAIT_FAILED)
		{
			InfoLog(<<"WaitForSingleObject is failing");
		}
//		ms_mutex_lock(&s->lock);
	}
//	ms_mutex_unlock(&s->lock);
	timeKillEvent (timerId);
	CloseHandle (TimeEvent);
	TimeEvent=NULL;
	unset_high_prio(precision);
	InfoLog(<<"MSTicker thread exiting");
//	ms_thread_exit(NULL);
//	return NULL;
}

#endif

int MSTicker::set_high_prio(void)
{
	int precision=2;
#ifdef WIN32
	MMRESULT mm;
	TIMECAPS ptc;
	mm=timeGetDevCaps(&ptc,sizeof(ptc));
	if (mm==0){
		if (ptc.wPeriodMin<precision)
			ptc.wPeriodMin=precision;
		else
			precision = ptc.wPeriodMin;
		mm=timeBeginPeriod(ptc.wPeriodMin);
		if (mm!=TIMERR_NOERROR){
		}
	}else{
	}

	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST)){
	}
#endif
	return precision;
}

void MSTicker::unset_high_prio(int precision)
{
#ifdef WIN32
	MMRESULT mm;

	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL)){
	}

	mm=timeEndPeriod(precision);
#endif
}

void MSTicker::sleepMs(int ms)
{
#ifdef WIN32
	Sleep(ms);
#else
	struct timespec ts;
	ts.tv_sec=0;
	ts.tv_nsec=ms*1000000LL;
	nanosleep(&ts,NULL);
#endif
}
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\msticker.cpp
*********************************************************************/

