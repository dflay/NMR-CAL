#ifndef SIS_INTERFACE_H
#define SIS_INTERFACE_H 

// interface to talk to an SIS digitizer

#include <cstdlib> 
#include <iostream>

#include "sisDigitizer.h"
#include "CommDriver.hh"

#include "sis3302_var.h"
#include "sis3316_var.h"

namespace SISInterface { 

   extern bool isDebug; 

   int open_connection(int type,const char *device_name,const char *device_path);
   int close_connection(int type,int handle);
   int get_module_id(int handle,u_int32_t addr,int &modID,int &majRev,int &minRev);

   int initialize(int handle,sisDigitizer_t myADC);        // generic call the user interacts with  
   int initialize_3302(int handle,sisDigitizer_t myADC);   // for the 3302    
   int initialize_3316(int handle,sisDigitizer_t myADC);   // for the 3316
    
   


} //::SISInterface  

#endif 
