#include <iostream>

#include "ProcessorChain.h"
//#include "RequestContext.hxx"

#include "rutil/Logger.hxx"
#include "rutil/Inserter.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::APP

using namespace resip;
using namespace std;


ProcessorChain::ProcessorChain()
{
//   DebugLog(<< "Instantiating new monkey chain " << this );
}

ProcessorChain::~ProcessorChain()
{
   DebugLog (<< "Deleting Chain: " << this);
   for (Chain::iterator i = mChain.begin(); i != mChain.end(); ++i)
   {
//      DebugLog(<< "Deleting RP: " << *i << " : " << **i);
      delete *i;
   }
   mChain.clear();
}

void
ProcessorChain::addProcessor(auto_ptr<Processor> rp)
{
//   DebugLog(<< "Adding new monkey to chain: " << *(rp.get()));
 
   mChain.push_back(rp.release());
}

void 
ProcessorChain::remProcessor(Processor* rp)
{
	for (Chain::iterator i = mChain.begin(); i != mChain.end(); ++i)
	{
		if((*i)==rp)
		{
			Chain::iterator tmp=i;
			i++;
			delete (*tmp);
			mChain.erase(tmp);
			return;
		}
	}
}

void
ProcessorChain::process(MediaContext &context)
{
//   DebugLog(<< "Monkey handling request: " << *this << "; reqcontext = " << rc);

   unsigned int position=0;

   for (; (position >=0 && position < mChain.size()); ++position)
   {
//      DebugLog(<< "Chain invoking monkey: " << *(mChain[position]));

      mChain[position]->process(context);
   }
   //DebugLog(<< "Monkey done processing: " << *(mChain[position]));
}


/* ====================================================================
 * 
 * ====================================================================
 * 
 *
 */
