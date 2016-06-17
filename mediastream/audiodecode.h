/*********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: audiocode
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\audideocode.h
*********************************************************************/


#ifndef AudioDecode_H 

#define AudioDecode_H 

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
// audiocode.h                                                                  
//----------------------------------------------------------------------------

//## package Default 


//## class AudioCode 
class AudioDecode: public Processor
{

////    Constructors and destructors    ////
public :
    
    //## auto_generated 
    AudioDecode(unsigned char pt=0xff);
    
    //## auto_generated 
    virtual ~AudioDecode();

////    Operations    ////
public :

	virtual void process(MediaContext &mc);
    //## operation process(msfilter) 
//	void encode(resip::Data& data);

//	void decode(resip::Data& data);

protected:
	unsigned char type;
	queue_t payload;
	BaseCodec *codecpcma;
	BaseCodec *codecpcmu;
	BaseCodec *codecgsm;
	BaseCodec *codecilbc;
	BaseCodec *codecg723;
	BaseCodec *codecg729;
};


#endif  
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\audiocode.h
*********************************************************************/
