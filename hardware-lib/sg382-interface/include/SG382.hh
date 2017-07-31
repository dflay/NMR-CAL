#ifndef SG382_H
#define SG382_H

#include <cstdlib>
#include <string> 

#include "CommDriver.hh"
#include "Device.hh"
#include "SG382Interface.hh"

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
