/*********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: fileplayer
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\fileplayer.h
*********************************************************************/


#ifndef FilePlayer_H 

#define FilePlayer_H 

#include <fstream>
#include <stdlib.h>
#include "waveheader.h"
#include "MediaContext.h"
#include "processor.h"
#include "rutil/Data.hxx"
//----------------------------------------------------------------------------
// fileplayer.h                                                                  
//----------------------------------------------------------------------------

//## package Default 


//## class fileplayer 
class FilePlayer: public Processor
{


////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    FilePlayer(int val = 20);
    
    //## auto_generated 
    virtual ~FilePlayer();


////    Operations    ////
public :
    
	virtual void process(MediaContext &mc);
    //## operation initial(msfilter) 
	int fileopen(resip::Data name);

    //## operation process(msfilter) 
//protected:
	std::fstream file;
	int codectype;
	int byteps;/*byte/s*/
	int bitpersample;/*byte per sample*/
	int rate;/*sample rate*/
	int nchannels;/*channel*/
	int hsize;
	int filelen;
	int loop_after;
	int pause_time;
	int interval;
	mblk_t *rb;
//	resip::Data rb;
};


#endif  
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\fileplayer.h
*********************************************************************/

