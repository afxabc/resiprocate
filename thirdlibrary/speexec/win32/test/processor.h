#ifndef Processor_H
#define Processor_H

//#include <iosfwd>
//#include <vector>
#include "mediacontext.h"

class Processor
{
public:
	Processor();
	virtual ~Processor();

	virtual void process(MediaContext& mc)=0;

protected:
//	MSTicker *ticker;
};

#endif

/* ====================================================================
 * ====================================================================
 * 
 *
 */
