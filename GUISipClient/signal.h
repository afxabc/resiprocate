#ifndef BASE_SIGNAL_H_
#define BASE_SIGNAL_H_

#include "timestamp.h"

class Signal
{
#ifndef WIN32
	typedef pthread_cond_t HANDLE;
#endif

public:
	Signal(bool on = false, bool auto_off = true);
	~Signal();

	void wait();			//���޵ȴ�
	bool wait(MicroSecond delay);

	void on();
	void off();

private:
	bool auto_;
	HANDLE handle_;
	bool signal_;
#ifndef WIN32
	pthread_mutex_t mutex_;
#endif

};

#endif
