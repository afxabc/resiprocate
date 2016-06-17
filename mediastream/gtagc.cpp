#include "gtagc.h"
#include <math.h>

Agc::Agc(int vol, int peek_level, int  dynamic_threshold, int noise_threshold): agcOn(false)
{
	SetGtagc(peek_level, dynamic_threshold, noise_threshold);
	volume = vol;
}

Agc::~Agc()
{
}

int Agc::SetGtagc(int peek_level, int  dynamic_threshold, int noise_threshold)
{
	int dynamic, noise;

	sample_max = 1;
	gain = 1.0f;
	silence_counter = 0;
    counter = 0;	

	//float a = pow( 10, ( (float)peek_level / 20.0 ) )/10;
	    //peak = REF_0dB * a;
	peak   = REF_0dB  * pow(10, ( (float)peek_level / 10.0 ));
	//float b = pow( 10, ( (float) dynamic_threshold / 20.0) )/10;
	    //dynamic = REF_0dB * b;
	dynamic= REF_0dB * pow(10, ( (float)dynamic_threshold / 10.0 ));
	//float c = pow( 10, ( (float) noise_threshold / 20.0) )/10;
	    //noise = REF_0dB * c;
	noise  = REF_0dB * pow(10, ( (float)noise_threshold / 10.0 ));
	
	//the second threshold
	active_threshold = dynamic;  

	//the third threshold
	silence_threshold = noise; 
	
	/*scale rate MAX > PEAK > active > silence*/
	pk = (float)(peak - active_threshold) / (MAX24bit - silence_threshold);

    return 1;
}

void Agc::DoGtagc(short *buffer, int len)
{
    int i;
	float gain_new;
    short sample, sample1;

	for(i=0;i<len;i++)
    {
		//index=i&0x00000001;
		//convert from 24bit to 32bit	
		//buffer[i]=( buffer[i] >= MAX24bit ? (buffer[i] | 0xff000000) : buffer[i]);
		sample = buffer[i];        
		sample = (sample < 0 ? -(sample):sample);

		if(sample > (short)sample_max)
        {
            sample_max = (short)sample;
        }

        counter ++;

        //attack
        if ((float)sample * gain > peak)
        {
            gain = (float)(peak / (float)sample_max) * 0.95f;
            silence_counter = 0;
            buffer[i] = (short) (buffer[i] * gain);
            continue;
        }
		
		//it's time to update the gain value
		if (counter >= 1600) 
        {			
			if ((short)(sample_max) > silence_threshold) 
			{
				 gain_new = 
				    0.95 * ( (float)pk * (sample_max-silence_threshold) + active_threshold ) / ((float)sample_max);
				
                
				if (silence_counter > 40) 
					gain += (gain_new - gain) * 0.5f;
				else
					gain += (gain_new - gain) * 0.1f;

				silence_counter = 0;
			}
			else
			{
				silence_counter++;	                	
				if ((gain > 1.0f) && (silence_counter >= 20))
              	    gain *= 0.8f; 
				continue;
           	}
          	counter = 0;   
			sample_max = 1;
		}//end if 

		//if (buffer[i] > silence_threshold/3)
		buffer[i] = (short) (buffer[i] * gain);

    }//end for

	for(i=0;i<len;i++)
	{	
		sample1 = buffer[i];        
		sample1 = (sample1 < 0 ? -(sample1):sample1);

        if(sample1 > (short)sample_max)
        {
            sample_max = (short)sample1;
        }

		if(volume==80) buffer[i]=0x00000000;
		else if(volume== 20) buffer[i]=buffer[i]*0.1;
		else if(volume== 1) buffer[i]=buffer[i]*0.9;
		else if(volume== 2) buffer[i]=buffer[i]*0.8;
		else if(volume== 3) buffer[i]=buffer[i]*0.7;
		else if(volume== 4) buffer[i]=buffer[i]*0.63;
		else if(volume== 5) buffer[i]=buffer[i]*0.56;
		else if(volume== 6) buffer[i]=buffer[i]*0.5;
		else if(volume== 7) buffer[i]=buffer[i]*0.45;
		else if(volume== 8) buffer[i]=buffer[i]*0.4;
		else if(volume== 9) buffer[i]=buffer[i]*0.35;
		else if(volume== 10) buffer[i]=buffer[i]*0.31;
		else if(volume== 11) buffer[i]=buffer[i]*0.28;
		else if(volume== 12) buffer[i]=buffer[i]*0.25;
		else if(volume== 13) buffer[i]=buffer[i]*0.22;
		else if(volume== 14) buffer[i]=buffer[i]*0.2;
		else if(volume== 15) buffer[i]=buffer[i]*0.18;
		else if(volume== 16) buffer[i]=buffer[i]*0.16;
		else if(volume== 17) buffer[i]=buffer[i]*0.14;
		else if(volume== 18) buffer[i]=buffer[i]*0.13;
		else if(volume== 19) buffer[i]=buffer[i]*0.11;
		else if(volume== 0) buffer[i]=buffer[i];
	}
    
    return;
}
void Agc::process(MediaContext &mc)
{
	mblk_t* inm = NULL;

	if (false == agcOn)
	{
		return;
	}

	for (inm=qbegin(&mc.payload0); !qend(&mc.payload0,inm); inm=qnext(&mc.payload0,inm))
	{
		int len = (inm->b_wptr - inm->b_rptr) / 2;
		DoGtagc((short*)(inm->b_rptr), len);       
	}
}

void Agc::OpenAgc()
{
	agcOn = true;
}

void Agc::CloseAgc()
{
	agcOn = false;
}
