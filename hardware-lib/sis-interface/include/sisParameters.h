#ifndef SIS_PARAMETERS_H
#define SIS_PARAMETERS_H

// data structure for SIS digitizer parameters  

typedef struct sisParameters { 
   u_int32_t moduleBaseAddress;      // address of digitizer (numbers from the two swtiches on the device)  
   double clockFrequency;            // clock frequency in Hz [computed using input units below]
   double signalLength;              // time duration of anticipated signal in sec 
   int outputUnits;                  // ADCCounts or Voltage
   int clockFreqUnits;               // [INPUT] kHz, MHz, GHz 
   int signalLengthUnits;            // [INPUT] usec, msec, sec
   int moduleID;                     // 3302 or 3316 
   int channelNumber;                // channel number to read out 
   int numberOfEvents;               // number of events 
   int numberOfSamples;              // number of samples; NOTE: this is different from the number of events! 
   int clockType;                    // 0 = internal, 1 = external; default is internal  
   int multiEventState;              // 0 = disabled, 1 = enabled; default is disabled 
   bool debug;                       // debug state 

   // default constructor
   sisParameters(u_int32_t addr=0x0,
                double clkFreq=0,double sigLen=0,int ounits=0,
                int clkUnits=0,int sigUnits=0,int modID=0,int chNum=1,int nev=1,int nsmpl=0,int clkType=0,int mev=0,bool d=false):
      moduleBaseAddress(addr),clockFrequency(clkFreq),signalLength(sigLen),
      outputUnits(ounits),clockFreqUnits(clkUnits),signalLengthUnits(sigUnits),moduleID(modID),channelNumber(chNum),numberOfEvents(nev),
      numberOfSamples(nsmpl),clockType(clkType),multiEventState(mev),debug(d) {} 

} sisParameters_t;

#endif   
