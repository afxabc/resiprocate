/*********************************************************************
	Rhapsody	: 7.0 
	Login		: yudongbo
	Component	: DefaultComponent 
	Configuration 	: DefaultConfig
	Model Element	: msrtp
//!	Generated Date	: Mon, 22, Jun 2009  
	File Path	: DefaultComponent\DefaultConfig\msrtp.h
*********************************************************************/


#ifndef msrtp_H 

#define msrtp_H 

#include <stdlib.h>
#include "g711common.h"
#include "rutil/Data.hxx"
#include "jrtplib/rtpsession.h"
#include "jrtplib/rtppacket.h"

//----------------------------------------------------------------------------
// msrtp.h                                                                  
//----------------------------------------------------------------------------

//## package Default 


//## class msrtp 
class msrtp
{

////    Constructors and destructors    ////
public :
    
    //## auto_generated 
	msrtp(RTPSession* session,unsigned char type);
    
    //## auto_generated 
    virtual ~msrtp();

////    Operations    ////
public :

    //## operation process(msfilter) 
	void rtpsend(resip::Data& data);

	resip::Data& rtprecv(RTPPacket *pack);

protected:
	RTPSession* sess;
	unsigned char mime_type;
	resip::Data data;

};


#endif  
/*********************************************************************
	File Path	: DefaultComponent\DefaultConfig\audiocode.h
*********************************************************************/
