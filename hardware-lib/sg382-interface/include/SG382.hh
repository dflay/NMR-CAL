#ifndef SG382_H
#define SG382_H

#include <cstdlib>
#include <string> 

#include "CommDriver.hh"
#include "Device.hh"

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

   const static uint DISABLE              = 0x0;
   const static uint ENABLE_AMPL_ONLY     = 0x1;
   const static uint ENABLE_FREQ_ONLY     = 0x2;
   const static uint ENABLE_AMPL_AND_FREQ = 0x3;

}

class SG382: public Device{

   public:
      SG382();
      ~SG382();

      int SetFrequency(double freq);
      int SetNTypeAmp(double amp);
      int SetBNCAmp(double amp);
      int SetBNCOutputState(int flag);
      int SetNTypeOutputState(int flag);
      int SetModulationState(int flag);
      int SetModulationRate(double freq);
      int SetModulationFunction(int flag);

      int GetBNCOutputState(int &state);
      int GetNTypeOutputState(int &state);
      int GetBNCAmplitude(double &amp);
      int GetNTypeAmplitude(double &amp);
      int GetFrequency(double &freq);

      int CheckErrors(char *err_msg);

};

#endif 
