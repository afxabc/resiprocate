/*********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: msticker
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\msticker.h
*********************************************************************/


#ifndef MSTicker_H 

#define MSTicker_H 

#include "rutil/ThreadIf.hxx"
#include "rutil/Mutex.hxx"
#include "processorchain.h"
#include "mediacontext.h"

#define TEST_VERSION
//----------------------------------------------------------------------------
// msticker.h                                                                  
//----------------------------------------------------------------------------
using namespace resip;
//## class msticker 
class MSTicker: public ThreadIf
{

////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    MSTicker(int val = 20);
    
    //## auto_generated 
    ~MSTicker();

	virtual void thread();

	void listExecute();

	void restart();
    //## auto_generated 
    int getinterval() const;
	void setinterval(int val);

	int set_high_prio(void);
	void unset_high_prio(int precision);
	void sleepMs(int ms);

	static unsigned __int64 get_cur_time();

public:
	mutable Mutex mutex;		//## link itsMsfilter 
	ProcessorChain executechain;
	MediaContext context;
	int interval;
	int ticks;
	unsigned __int64 time;/* a time since the start of the ticker expressed in milisec*/
	unsigned __int64 orig;
	bool running;
#ifdef WIN32_TIMERS
	HANDLE TimeEvent;
#endif
#ifdef TEST_VERSION
	int expired;
#endif
};


#endif  
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\msticker.h
*********************************************************************/

