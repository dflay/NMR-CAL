#ifndef SG382_INTERFACE_HH
#define SG382_INTERFACE_HH

// Library to control Stanford Research Systems RG382 
// RF Signal Generator over USB->SG382 port

#include <cstdlib>
#include <iostream>
#include <string>
#include <math.h> 
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

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
   struct termios old_termios;

   int open_connection(const char *device_path);
   int close_connection(int rs232_handle);
   int clear_error(int rs232_handle); 
   int write_cmd(int rs232_handle,char *buffer);
   int ask(int rs232_handle,char *in_buffer,char *out_buffer);

   int set_freq(int rs232_handle,double freq); 
   int set_ntype_amp(int rs232_handle,double amp); 
   int set_bnc_amp(int rs232_handle,double amp); 
   int set_bnc_output_state(int rs232_handle, int flag);
   int set_ntype_output_state(int rs232_handle, int flag);
   int set_modulation_state(int rs232_handle, int flag);
   int set_modulation_rate(int rs232_handle, double freq);
   int set_modulation_function(int rs232_handle, int flag);
   
   int get_bnc_output_state(int rs232_handle,int &state); 
   int get_ntype_output_state(int rs232_handle,int &state); 
   int get_bnc_amplitude(int rs232_handle,double &amp);  
   int get_ntype_amplitude(int rs232_handle,double &amp);  
   int get_frequency(int rs232_handle,double &freq);  
   int get_error(int rs232_handle,char *response);

}

#endif
