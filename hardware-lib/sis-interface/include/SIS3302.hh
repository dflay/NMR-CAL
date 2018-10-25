#ifndef SIS3302_H
#define SIS3302_H 

// implementation of SIS3302 class

#include "SISBase.hh"
#include "sis3302_var.h"

class SIS3302: public SISBase { 

   public:
      SIS3302( sisParameters_t par=sisParameters() );
      ~SIS3302(); 

      int Initialize();
      int ReInitialize();
      int ReadOutData(std::vector<double> &outData);

   private: 
      int set_clock_freq(int vme_handle,int clock_state,int freq_mhz);

}; 

#endif 
