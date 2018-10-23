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

#define MAX_NUMBER_LWORDS_64MBYTE 0x1000000       // 64MB 

namespace SISInterface { 
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
      virtual int ReadOutData(); 

      int GetModuleID();

      void SetParameters( sisParameters_t par = sisParameters() ); 
      void SetModuleBaseAddress(u_int32_t addr) { fParameters.moduleBaseAddress = addr;  }
      void SetClockFrequency(double f)          { fParameters.clockFrequency    = f;     } 
      void SetClockPeriod(double t)             { fParameters.clockPeriod       = t;     }
      void SetSignalLength(double x)            { fParameters.signalLength      = x;     } 
      void SetModuleID(int modID)               { fParameters.moduleID          = modID; }
      void SetChannelNumber(int ch)             { fParameters.channelNumber     = ch;    } 
      void SetNumberOfEvents(int nev)           { fParameters.numberOfEvents    = nev;   }   
      void SetNumberOfSamples(int s)            { fParameters.numberOfSamples   = s;     }  
      void SetClockType(int t)                  { fParameters.clockType         = t;     }
      void SetMultiEventStatus(int t)           { fParameters.multiEventState   = t;     }  
      void SetDebugMode(bool t)                 { fParameters.debug             = t;     }
     
      int GetData(std::vector<unsigned short> &x) const;   // return the data to the application  

   protected:
      sisParameters_t fParameters;
      std::vector<unsigned short> fData;        // this is the data read out by the ADC

      void ClearData()                          { fData.clear(); } 

};

#endif 
