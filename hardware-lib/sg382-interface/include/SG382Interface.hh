#ifndef SG382_INTERFACE_HH
#define SG382_INTERFACE_HH

// Library to control Stanford Research Systems SG382 RF Signal Generator 

#include <cstdlib>
#include <iostream>
#include <string>
#include <math.h> 
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "CommDriver.hh"

#define SG382_DISABLE              0x0                 
#define SG382_ENABLE_AMPL_ONLY     0x1  
#define SG382_ENABLE_FREQ_ONLY     0x2  
#define SG382_ENABLE_AMPL_AND_FREQ 0x3

namespace sg382_interface { 

   enum outputState{ 
      kDISABLE = 0,
      kENABLE  = 1 
   };

   enum modFunc{
      kSINE     = 0,
      kRAMP     = 1,
      kTRIANGLE = 2,
      kSQUARE   = 3,
      kNOISE    = 4,
      kEXTERNAL = 5
   };  

   const int SG382_SLEEP_TIME = 100; 

   int open_connection(int type,const char *device_path);
   int close_connection(int type,int handle);
   int clear_error(int type,int handle); 

   int set_freq(int type,int handle,double freq); 
   int set_ntype_amp(int type,int handle,double amp); 
   int set_bnc_amp(int type,int handle,double amp); 
   int set_bnc_output_state(int type,int handle, int flag);
   int set_ntype_output_state(int type,int handle, int flag);
   int set_modulation_state(int type,int handle, int flag);
   int set_modulation_rate(int type,int handle, double freq);
   int set_modulation_function(int type,int handle, int flag);
   
   int get_bnc_output_state(int type,int handle,int &state); 
   int get_ntype_output_state(int type,int handle,int &state); 
   int get_bnc_amplitude(int type,int handle,double &amp);  
   int get_ntype_amplitude(int type,int handle,double &amp);  
   int get_frequency(int type,int handle,double &freq);  
   int get_error(int type,int handle,char *response);

}

#endif
