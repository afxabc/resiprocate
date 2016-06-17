#if !defined(ProcessorPhain_H)
#define ProcessorPhain_H

#include <memory>
#include <vector>
#include "processor.h"


class ProcessorChain : public Processor
{
   public:
      ProcessorChain();
      virtual ~ProcessorChain();

      void addProcessor(std::auto_ptr<Processor>);
	  void remProcessor(Processor* rp);
	  int getChainSize() {return mChain.size();};

      virtual void process(MediaContext& context);

      typedef std::vector<Processor*> Chain;
      
   private:
      Chain mChain;

};

#endif

/* ====================================================================
 * ====================================================================
 * 
 *
 */
