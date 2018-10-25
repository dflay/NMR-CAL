#ifndef SIS_BASE_H
#define SIS_BASE_H 

// ABC for SIS digitizers  

#include <cstdlib> 
#include <iostream>
#include <string> 
#include <vector> 
#include <math.h>

#include "sisParameters.h"
#include "CommDriver.hh"
#include "Device.hh"

#include "sis1100_var.h"
#include "sis3100_vme_calls.h"

#define MAX_NUMBER_LWORDS_64MBYTE 0x1000000 // 64MB 

namespace SISInterface {
   enum freqUnits{
      Hz  = 0,
      kHz = 1,
      MHz = 2,
      GHz = 3
   };

   enum timeUnits{
      sec  = 0,
      msec = 1,
      usec = 2,
      nsec = 3 
   };

   enum amplUnits{
      kADCCounts = 0,
      kVolts     = 1
   }; 
 
   enum clkType{
      kInternal = 0,
      kExternal = 1
   };

   enum sisSwitch{
      kDisable = 0,
      kEnable  = 1
   };
}

class SISBase: public Device {

   public: 
      SISBase( sisParameters_t par=sisParameters() );
      virtual ~SISBase();

      virtual int Initialize();
      virtual int ReInitialize(); 
      virtual int ReadOutData(std::vector<double> &x); 

      int ReadModuleID();
      int GetModuleID()                   const { return fParameters.moduleID; }

      void SetParameters( sisParameters_t par = sisParameters() ); 
      void SetModuleBaseAddress(u_int32_t addr) { fParameters.moduleBaseAddress = addr;  }
      void SetOutputUnits(int u)                { fParameters.outputUnits       = u;     } 
      void SetChannelNumber(int ch)             { fParameters.channelNumber     = ch;    } 
      void SetNumberOfEvents(int nev)           { fParameters.numberOfEvents    = nev;   }   
      void SetClockType(int t)                  { fParameters.clockType         = t;     }
      void SetMultiEventStatus(int t)           { fParameters.multiEventState   = t;     }  
      void SetDebugMode(bool t)                 { fParameters.debug             = t;     }
      void SetClockFrequency(double f,int units=SISInterface::Hz); 
      void SetSignalLength(double x,int units=SISInterface::sec); 

   protected:
      sisParameters_t fParameters;
      
      double ConvertToVoltage(unsigned short x); 

};

#endif 
