#ifndef SIS3316_H
#define SIS3316_H 

// implementation of SIS3316 class

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

#include "sisParameters.h"
#include "SISBase.hh"
#include "Device.hh"
#include "CommDriver.hh"

#include "sis3316_var.h"

class SIS3316: public SISBase { 

   public:
      SIS3316( sisParameters_t par=sisParameters() );
      ~SIS3316():

      int Initialize();
      int ReInitialize();
      int ReadOutData(); 

}; 

#endif 
