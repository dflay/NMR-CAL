#include "SIS3302.hh"
//______________________________________________________________________________
SIS3302::SIS3302(sisParameters_t par){
   fName = "SIS3302"; 
   SetParameters(par); 
}
//______________________________________________________________________________
SIS3302::~SIS3302(){

}
//______________________________________________________________________________
int SIS3302::Initialize(){

   // Initialize (or reset) the SIS3302 to NMR signal-gathering configuration
   // Note: This is separated from the general SISInit() because 
   //       we want to call this function many times in the main 
   //       part of the code to reset the ADC memory after every block read, 
   //       but don't want to waste time re-reading the input file for the ADC. 

   int vme_handle      = fHandle;
   bool isDebug        = fParameters.debug;
   u_int32_t base_addr = fParameters.moduleBaseAddress;  
   u_int32_t addr      = 0x0;  

   int rc=0;
   if(isDebug) std::cout << "[SIS3302::Initialize]: Configuring..." << std::endl;

   u_int32_t data32 = 0;

   // reset StruckADC to power-up state and clear all sampled data from memory
   if(isDebug) std::cout << "[SIS3302::Initialize]: Issuing key reset..." << std::endl;
   addr = base_addr + SIS3302_KEY_RESET; 
   rc = CommDriver::vme_write32(vme_handle,addr,0x0);
   if(isDebug) std::cout << "[SIS3302::Initialize]: --> Done" << std::endl;

   rc = ReadModuleID();

   int multiEventState = fParameters.multiEventState; 

   // general configuration settings
   if(multiEventState==0){
      // multi-event state disabled
      std::cout << "[SIS3302::Initialize]: ADC in SINGLE-EVENT mode. \n" << std::endl;
      data32 = SIS3302_ACQ_DISABLE_LEMO_START_STOP
         + SIS3302_ACQ_DISABLE_AUTOSTART
         + SIS3302_ACQ_DISABLE_MULTIEVENT;
   }else if(multiEventState==1){
      // multi-event state enabled
      std::cout<< "[SIS3302::Initialize]: ADC in MULTI-EVENT mode. \n" << std::endl;
      data32 = SIS3302_ACQ_DISABLE_LEMO_START_STOP
         + SIS3302_ACQ_DISABLE_AUTOSTART
         + SIS3302_ACQ_ENABLE_MULTIEVENT;
   }else{
      std::cout << "[SIS3302::Initialize]: ADC event mode not properly set!  Defaulting to single-event mode..." << std::endl;
      data32 = SIS3302_ACQ_DISABLE_LEMO_START_STOP
         + SIS3302_ACQ_DISABLE_AUTOSTART
         + SIS3302_ACQ_DISABLE_MULTIEVENT;
   }

   if(isDebug) std::cout << "[SIS3302::Initialize]: Applying settings..." << std::endl;
   addr = base_addr + SIS3302_ACQUISITION_CONTROL; 
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(isDebug) std::cout << "[SIS3302::Initialize]: --> Done" << std::endl;

   int ClockType = fParameters.clockType;
   int ClockFreq = (int)fParameters.clockFrequency;
   int units     = fParameters.clockFreqUnits;
   int ClockFreq_in_units=0;

   if(units==SISInterface::kHz) ClockFreq_in_units = ClockFreq/1E+3;
   if(units==SISInterface::MHz) ClockFreq_in_units = ClockFreq/1E+6;
   if(units==SISInterface::GHz) ClockFreq_in_units = ClockFreq/1E+9;

   rc = set_clock_freq(vme_handle,ClockType,ClockFreq_in_units);

   if(isDebug) std::cout << "[SIS3302::Initialize]: Setting start/stop delays..." << std::endl;
   addr = base_addr + SIS3302_START_DELAY; 
   rc = CommDriver::vme_write32(vme_handle,addr,0); // TODO: later use this to cut out a few of the initial noisy usec of probe signal
   addr = base_addr + SIS3302_STOP_DELAY; 
   rc = CommDriver::vme_write32(vme_handle,addr,0);  // we almost certainly will never need a stop delay
   if(isDebug) std::cout << "[SIS3302::Initialize]: --> Done " << std::endl;

   data32 = SIS3302_EVENT_CONF_ENABLE_SAMPLE_LENGTH_STOP; // enable automatic event stop after a certain number of samples
   addr = base_addr + SIS3302_EVENT_CONFIG_ALL_ADC;
   rc = CommDriver::vme_write32(vme_handle,addr,data32);

   if(isDebug) std::cout << "[SIS3302::Initialize]: Configuration complete." << std::endl;

   addr = base_addr + SIS3302_KEY_ARM; 
   rc = CommDriver::vme_write32(vme_handle,addr,0x0);

   return rc;
}
//______________________________________________________________________________
int SIS3302::ReInitialize(){
   // re-initialize the digitizer in the case that the event length has changed

   bool isDebug        = fParameters.debug;

   int vme_handle      = fHandle;
   u_int32_t base_addr = fParameters.moduleBaseAddress; 
   u_int32_t addr      = 0x0; 

   // now set the number of samples recorded before each event stops
   // the number of events is the "larger unit" compared to 
   // number of samples... that is, 1 event = N samples.  
   double signal_length   = fParameters.signalLength;
   double event_length_f  = signal_length*fParameters.clockFrequency; // total samples per event = time_of_signal*sample_rate 
   u_int32_t event_length = (u_int32_t)event_length_f;
   int event_length_int   = (int)event_length_f;

   char msg[512]; 
   // sprintf(msg,"[SISInterface::reinitialize_3302]: signal_length = %.0lf, sampling period = %.3E, event_length = %.0lf",signal_length,sampling_period,event_length_f);
   // std::cout << msg << std::endl; 

   sprintf(msg,"[SIS3302::ReInitialize]: Setting up to record %d samples per event...",event_length_int);
   if(isDebug) std::cout << msg << std::endl;

   // set the event length 
   u_int32_t data32 = (event_length - 4) & 0xfffffC;       // what is this wizardry? no idea, from StruckADC manual.
   addr = base_addr + SIS3302_SAMPLE_LENGTH_ALL_ADC; 
   int rc = CommDriver::vme_write32(vme_handle,addr,data32);

   int NumberOfEvents = fParameters.numberOfEvents;

   int PULSES_PER_READ = 5; 

   addr = base_addr + SIS3302_MAX_NOF_EVENT; 
   if(NumberOfEvents>PULSES_PER_READ){
      rc = CommDriver::vme_write32(vme_handle,addr,PULSES_PER_READ);
   }else{
      rc = CommDriver::vme_write32(vme_handle,addr,NumberOfEvents);
   }

   if(isDebug) std::cout << "[SISInterface::reinitialize_3302]: Configuration complete. " << std::endl;

   addr = base_addr + SIS3302_KEY_ARM;
   rc = CommDriver::vme_read32(vme_handle,addr,0x0);

   return rc;
} 
//______________________________________________________________________________
int SIS3302::ReadOutData(std::vector<double> &outData){

   // read out a single pulse to the fData vector  

   bool isDebug             = fParameters.debug;  

   int vme_handle           = fHandle;
   int chNumber             = fParameters.channelNumber;
   u_int32_t base_addr      = fParameters.moduleBaseAddress; 

   const int NUM_SAMPLES    = fParameters.numberOfSamples;
   u_int32_t NUM_SAMPLES_ul = (u_int32_t)NUM_SAMPLES;
   outData.resize(NUM_SAMPLES); 

   u_int32_t *data32       = (u_int32_t *)malloc( sizeof(u_int32_t)*NUM_SAMPLES );

   // block read of data from ADC
   // gettimeofday(&gStart,NULL);
   u_int32_t addr=0x0;
   if(chNumber==1){
     addr = base_addr + SIS3302_ADC1_OFFSET;
   }else if(chNumber==2){
     addr = base_addr + SIS3302_ADC2_OFFSET;
   }else if(chNumber==3){
     addr = base_addr + SIS3302_ADC3_OFFSET;
   }else if(chNumber==4){
     addr = base_addr + SIS3302_ADC4_OFFSET;
   }else if(chNumber==5){
     addr = base_addr + SIS3302_ADC5_OFFSET;
   }else if(chNumber==6){
     addr = base_addr + SIS3302_ADC6_OFFSET;
   }else if(chNumber==7){
     addr = base_addr + SIS3302_ADC7_OFFSET;
   }else if(chNumber==8){
     addr = base_addr + SIS3302_ADC8_OFFSET;
   }

   u_int32_t NumWords = 0;
   int rc = vme_A32_2EVME_read(vme_handle,addr,&data32[0],NUM_SAMPLES_ul/2,&NumWords);
   // gettimeofday(&gStop,NULL);
   // unsigned long delta_t = gStop.tv_usec - gStart.tv_usec;
 
   char msg[512]; 
   if( isDebug || rc!=0 ){
      if(rc==0){
         sprintf(msg,"[SIS3302::ReadOutData]: Block read return code = %d ",rc);
      }else{
         sprintf(msg,"[SIS3302::ReadOutData]: ERROR! Block read return code = %d, num words = %d",rc,NumWords);
      }
      std::cout << msg << std::endl;
   }

   u_int32_t data1, data2;
   u_int16_t d1,d2;
   double D1=0,D2=0;

   // convert to an array of unsigned shorts  
   for(int i=0;i<NUM_SAMPLES/2;i++){
      data1          =  data32[i] & 0x0000ffff;             // low bytes 
      data2          = (data32[i] & 0xffff0000)/pow(2,16);  // high bytes 
      d1             = (unsigned short)data1;
      d2             = (unsigned short)data2;
      if( fParameters.outputUnits==SISInterface::Volts ){
         D1 = ConvertToVoltage(d1); 
         D2 = ConvertToVoltage(d2); 
      }else{
         D1 = (double)d1;
         D2 = (double)d2;
      }
      outData[i*2]   = D1;
      outData[i*2+1] = D2;
   }

   return 0;
}
//______________________________________________________________________________
int SIS3302::set_clock_freq(int vme_handle,int clock_state,int freq_mhz){

   u_int32_t data32=0x0;

   int clock_select=0;

   std::string clockType = "UNKNOWN"; 

   switch(clock_state){
      case SISInterface::kInternal: // internal clock 
         switch(freq_mhz){
            case 1: 
               data32 = SIS3302_ACQ_SET_CLOCK_TO_1MHZ;
               clock_select = 1; 
               break;
            case 10: 
               data32 = SIS3302_ACQ_SET_CLOCK_TO_10MHZ;
               clock_select = 10; 
               break;
            case 25: 
               data32 = SIS3302_ACQ_SET_CLOCK_TO_25MHZ;
               clock_select = 25; 
               break;
            case 50: 
               data32 = SIS3302_ACQ_SET_CLOCK_TO_50MHZ;
               clock_select = 50; 
               break;
            case 100:
               data32 = SIS3302_ACQ_SET_CLOCK_TO_100MHZ;
               clock_select = 100; 
               break;
            default:
               data32 = SIS3302_ACQ_SET_CLOCK_TO_1MHZ;
               clock_select = 1; 
               break;
         }
         break;
      case SISInterface::kExternal: // external clock 
         data32 = SIS3302_ACQ_SET_CLOCK_TO_LEMO_CLOCK_IN;
         clock_select = freq_mhz;
         clockType = "EXTERNAL"; 
         break;
      default:
         data32 = SIS3302_ACQ_SET_CLOCK_TO_1MHZ;
         clock_select = 1;
         clockType = "INTERNAL"; 
   }

   std::cout << "[SIS3302::set_clock_freq]: Using an " << clockType << " set to " << clock_select << " MHz" << std::endl;

   u_int32_t base_addr = fParameters.moduleBaseAddress; 
   u_int32_t addr      = base_addr + SIS3302_ACQUISITION_CONTROL; 
   int rc              = CommDriver::vme_write32(vme_handle,addr,data32);
   return rc;
}

