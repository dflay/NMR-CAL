#ifndef SIS_PARAMETERS_H
#define SIS_PARAMETERS_H

// data structure for SIS digitizer parameters  

#include <string> 

typedef struct sisParameters { 
   u_int32_t moduleBaseAddress;      // address of digitizer (numbers from the two swtiches on the device)  
   std::string clockFreqUnits;       // kHz, MHz, GHz 
   std::string signalLengthUnits;    // us, ms, s
   double clockFrequency;            // clock frequency in Hz 
   double clockPeriod;               // clock period in sec 
   double signalLength;              // time duration of anticipated signal in sec 
   int moduleID;                     // 3302 or 3316 
   int channelNumber;                // channel number to read out 
   int numberOfEvents;               // number of events 
   int numberOfSamples;              // number of samples; NOTE: this is different from the number of events! 
   int clockType;                    // 0 = internal, 1 = external; default is internal  
   int multiEventState;              // 0 = disabled, 1 = enabled; default is disabled 
   bool debug;                       // debug state 

   // default constructor
   sisParameters(u_int32_t addr=0x0,std::string clkUnits="MHz",std::string sigUnits="s",
                double clkFreq=0,double clkPeriod=0,double sigLen=0,
                int modID=0,int chNum=0,int nev=0,int nsmpl=0,int clkType=0,int mev=0,bool d=false):
      moduleBaseAddress(addr),clockFreqUnits(clkUnits),signalLengthUnits(sigUnits),clockFrequency(clkFreq),
      clockPeriod(clkPeriod),signalLength(sigLen),moduleID(modID),channelNumber(chNum),numberOfEvents(nev),
      numberOfSamples(nsmpl),clockType(clkType),multiEventState(mev),debug(d) {} 

} sisParameters_t;

#endif   
