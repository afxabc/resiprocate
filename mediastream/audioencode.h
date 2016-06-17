/*********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: audiocode
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\AudioEncode.h
*********************************************************************/


#ifndef AudioEncode_H 

#define AudioEncode_H 

#include <stdlib.h>
#include "rutil/Data.hxx"
#include "MediaContext.h"
#include "processor.h"

#include "basecodec.h"
#include "codecg723.h"
#include "codecg729.h"
#include "codecgsm.h"
#include "codecilbc.h"
#include "codecpcm.h"

//----------------------------------------------------------------------------
// AudioEncode.h                                                                  
//----------------------------------------------------------------------------

//## package Default 


//## class AudioEncode 
class AudioEncode: public Processor
{

////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    AudioEncode(unsigned char pt=0);
    
    //## auto_generated 
    virtual ~AudioEncode();

////    Operations    ////
public :

	virtual void process(MediaContext &mc);
    //## operation process(msfilter) 
//	void encode(resip::Data& data);

//	void decode(resip::Data& data);

protected:
	unsigned char type;
	BaseCodec *codec;
};


#endif  
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\audiocode.h
*********************************************************************/
