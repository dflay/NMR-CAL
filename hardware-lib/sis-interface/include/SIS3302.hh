#ifndef SIS3302_H
#define SIS3302_H 

// implementation of SIS3302 class

#include <cstdlib> 
#include <iostream> 
#include <fstream> 
#include <string>

#include "sisParameters.h"
#include "SISBase.hh"
#include "Device.hh"
#include "CommDriver.hh"

class SIS3302: public SISBase { 

   public:
      SIS3302( sisParameters_t par=sisParameters() );
      ~SIS3302(); 

      int Initialize();
      int ReInitialize();
      int ReadOutData();

}; 

#endif 
