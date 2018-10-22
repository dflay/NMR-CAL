#ifndef SIS_DIGITIZER_H
#define SIS_DIGITIZER_H

// data structure for an SIS digitizer 

#include <string> 

typedef struct sisDigitizer { 
   std::string name;                   // name of the device 
   std::string clockFreqUnits;         // kHz, MHz, GHz 
   std::string signalLengthUnits;      // us, ms, s
   int vmeHandle;                      // VME ID 
   int modID;                          // module ID 
   int channelNumber;                  // channel number to read out 
   int numberOfEvents;                 // number of events 
   int NumberOfSamples;                // number of samples; NOTE: this is different from the number of events! 
   int clockType;                      // 0 = internal, 1 = external; default is internal  
   int multiEventState;                // 0 = disabled, 1 = enabled; default is disabled 
   double clockFrequency;              // clock frequency in Hz 
   double clockPeriod;                 // clock period in sec 
   double signalLength;                // time duration of anticipated signal in sec 
} sisDigitizer_t;

#endif   
