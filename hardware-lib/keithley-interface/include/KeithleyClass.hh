#ifndef KEITHLEY_CLASS_HH
#define KEITHLEY_CLASS_HH 

#include <cstdlib> 
#include <string> 

#include "CommDriver.hh"
#include "Device.hh"

// a class that derives from the ABC Device 

class Keithley: public Device{

   public: 
      int SetToRemoteMode(); 
      int SetRange(double maxRange); 
      
      int GetMode(char *response); 
      int GetResistance(double &R); 

      Keithley();
      ~Keithley();

}; 

#endif 
