#include "SISInterface.hh"
//______________________________________________________________________________
namespace SISInterface { 
   // global variables 
   bool isDebug = false; 
   //______________________________________________________________________________
   int open_connection(int type,const char *device_name,const char *device_path){
      int handle = CommDriver::open_connection(type,device_name,device_path);
      return handle; 
   }
   //______________________________________________________________________________
   int close_connection(int type,int handle){
      int rc = CommDriver::close_connection(type,handle);
      return rc; 
   }
   //______________________________________________________________________________
   int get_module_id(int handle,u_int32_t addr,int &modID,int &majRev,int &minRev){
      // read the device ID data 
      // modID  = module ID 
      // majRev = major revision 
      // minRev = minor revision 
      u_int32_t data=0;
      int rc = CommDriver::vme_read32(handle,addr,&data);
      modID  =  data >> 16;
      majRev = (data >> 8) & 0xff;  
      minRev =  data & 0xff;  
      return rc; 
   }
   //______________________________________________________________________________
   int initialize(sisDigitizer_t myADC){
      int rc=-1; 
      int modID = myADC.modID; 
      if(modID==3302){
         rc = initialize_3302(myADC); 
      }else if(modID==3316){
         rc = initialize_3316(myADC);
      }else{
         std::cout << "[SISInterface::initialize]: Invalid module ID " << modID << std::endl;
         return -1;
      } 
      return rc;
   }
   //______________________________________________________________________________
   int initialize_3302(sisDigitizer_t myADC){

      // Initialize (or reset) the SIS3302 to NMR signal-gathering configuration
      // Note: This is separated from the general SISInit() because 
      //       we want to call this function many times in the main 
      //       part of the code to reset the ADC memory after every block read, 
      //       but don't want to waste time re-reading the input file for the ADC. 

      int vme_handle = myADC.vmeHandle; 

      int rc=0;
      if(isDebug) std::cout << "[SISInterface::initialize_3302]: Configuring..." << std::endl;

      u_int32_t data32 = 0;

      // reset StruckADC to power-up state and clear all sampled data from memory
      if(isDebug) std::cout << "[SISInterface::initialize_3302]: Issuing key reset..." << std::endl;
      rc = CommDriver::vme_write32(vme_handle,SIS3302_KEY_RESET,0x0);
      if(isDebug) std::cout << "[SISInterface::initialize_3302]: --> Done" << std::endl;

      int modID=0,majRev=0,minRev=0;
      rc = get_module_id(vme_handle,SIS3302_MODID,modID,majRev,minRev);

      int multiEventState = myADC.multiEventState; 

      // general configuration settings
      if(multiEventState==0){
         // multi-event state disabled
         std::cout << "[SISInterface::initialize_3302]: ADC in SINGLE-EVENT mode. \n" << std::endl;
         data32 = SIS3302_ACQ_DISABLE_LEMO_START_STOP
            + SIS3302_ACQ_DISABLE_AUTOSTART
            + SIS3302_ACQ_DISABLE_MULTIEVENT;
      }else if(multiEventState==1){
         // multi-event state enabled
         std::cout<< "[SISInterface::initialize_3302]: ADC in MULTI-EVENT mode. \n" << std::endl;
         data32 = SIS3302_ACQ_DISABLE_LEMO_START_STOP
            + SIS3302_ACQ_DISABLE_AUTOSTART
            + SIS3302_ACQ_ENABLE_MULTIEVENT;
      }else{
         std::cout << "[SISInterface::initialize_3302]: ADC event mode not properly set!  Defaulting to single-event mode..." << std::endl;
         data32 = SIS3302_ACQ_DISABLE_LEMO_START_STOP
            + SIS3302_ACQ_DISABLE_AUTOSTART
            + SIS3302_ACQ_DISABLE_MULTIEVENT;
         myADC.multiEventState = 0;
      }
      if(isDebug) std::cout << "[SISInterface::initialize_3302]: Applying settings..." << std::endl;
      rc = CommDriver::vme_write32(vme_handle,SIS3302_ACQUISTION_CONTROL,data32);
      if(isDebug) std::cout << "[SISInterface::initialize_3302]: --> Done" << std::endl;
      return rc;
   }
   //______________________________________________________________________________
   int initialize_3316(sisDigitizer_t myADC){

      // initialization function for the 3316

      // double dt=0;
      // unsigned long *timeStart = (unsigned long *)malloc( sizeof(unsigned long)*6 );
      // unsigned long *timeEnd   = (unsigned long *)malloc( sizeof(unsigned long)*6 );

      // int i=0;
      // for(i=0;i<6;i++) timeStart[i] = 0;
      // for(i=0;i<6;i++) timeEnd[i]   = 0;

      // GetTimeStamp_usec(timeStart);

      u_int32_t data32=0;

      // general settings and data

      // input from user 
      int vme_handle    = myADC.vmeHandle;
      int use_ext_clock = myADC.clockType;                  // 0 => false; 1 => true 

      // some values that (most likely) won't change 
      unsigned int auto_trigger_enable = 1;                                 // 1 => use internal triggering; 0 => use external triggering 
      unsigned int analog_ctrl_val     = 0 ;                                // 5V Range
      // unsigned int analog_ctrl_val     = analog_ctrl_val + 0x01010101 ;  // set to 2V Range
      // unsigned int analog_ctrl_val     = analog_ctrl_val + 0x04040404 ;  // disable 50 Ohm Termination (enables 1k termination) 
      int adc_125MHz_flag              = 1;                                 // 0 => 250 MHz; 1 => 125 MHz; choosing the 250 MHz (125 MHz) option will use the 14-bit (16-bit) ADC  
      double trigger_gate_window_sec   = 80E-6;                             // choose the trigger gate window length (seconds); not sure if this matters for us...    

      // get some details about the clock we're using 
      int ClockFreq=0;
      if(adc_125MHz_flag==0) ClockFreq = 250E+6;
      if(adc_125MHz_flag==1) ClockFreq = 125E+6;
      if(use_ext_clock==1)   ClockFreq = (int)myADC.clockFrequency;

      unsigned int trigger_gate_window_length = (unsigned int)( trigger_gate_window_sec*ClockFreq );   // 

      char msg[512]; 

      if(isDebug){
         std::cout << "[SISInterface::initialize_3316]: Initializing..." << std::endl;
         sprintf(msg,"[SISInterface::initialize_3316]: Sampling frequency: %d Hz",ClockFreq);
         std::cout << msg << std::endl;
         sprintf(msg,"[SISInterface::initialize_3316]: Trigger gate window length: %u (%.4E sec) \n",
               trigger_gate_window_length,trigger_gate_window_sec);
         std::cout << msg << std::endl;
      }

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Reading the MOD ID..." << std::endl;
      int modID=0,majRev=0,minRev=0;
      int rc = get_module_id(vme_handle,SIS3316_MODID,modID,majRev,minRev);
      if( isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done."   << std::endl;
      if( isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed!" << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Issuing key reset..." << std::endl;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_KEY_RESET, 0x0);
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! " << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Issuing key disarm..." << std::endl;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_KEY_DISARM, 0x0);
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! " << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Configuring the ADC via SPI... " << std::endl;
      rc = adc_spi_setup(vme_handle,adc_125MHz_flag);
      if( isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if( isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! " << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Configuring the clock..." << std::endl;
      rc = configure_clock_3316(myADC,use_ext_clock,adc_125MHz_flag);
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done." << std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! " << std::endl;

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Turning on the ADC chip outputs... " << std::endl;
      u_int32_t an_offset=0;
      int fail=0;
      for(int i=0;i<4;i++){
         an_offset = i*SIS3316_FPGA_ADC_REG_OFFSET;
         rc  = CommDriver::vme_write32(vme_handle,an_offset + SIS3316_ADC_CH1_4_SPI_CTRL_REG, 0x01000000 ); // enable ADC chip outputs
         if(rc!=0) fail++;
      }
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done." << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed " << fail << " times!" << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Setting the LEMO output 'CO'..." << std::endl;
      data32 = 0x1 ; // Select Sample Clock
      rc = CommDriver::vme_write32(vme_handle,SIS3316_LEMO_OUT_CO_SELECT_REG, data32 ); //
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! " << std::endl;

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Enabling the LEMO output 'TO'..." << std::endl;
      data32 = 0xffff ; // Select all triggers
      rc = CommDriver::vme_write32(vme_handle,SIS3316_LEMO_OUT_TO_SELECT_REG, data32 ); //
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! " << std::endl;

      // header writes 
      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Setting up the headers..." << std::endl;
      fail = 0;
      data32 = 0x0 ;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG  , data32 ); //
      if(rc!=0) fail++;
      data32 = 0x00400000;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG  , data32 ); //
      if(rc!=0) fail++;
      data32 = 0x00800000 ;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG , data32 ); //
      if(rc!=0) fail++;
      data32 = 0x00C00000;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG, data32 ); //
      if(rc!=0) fail++;
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed " << fail << " times!" << std::endl;

      // gain/termination 
      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Setting the gain and termination options..." << std::endl;
      fail = 0;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_ANALOG_CTRL_REG  ,analog_ctrl_val);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_ANALOG_CTRL_REG  ,analog_ctrl_val);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_ANALOG_CTRL_REG ,analog_ctrl_val);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_ANALOG_CTRL_REG,analog_ctrl_val);
      if(rc!=0) fail++;
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done." << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed " << fail << " times! " << std::endl;

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Intializing the ADC (DAC) offsets... " << std::endl;
      u_int32_t adc_dac_offset=0;
      u_int32_t analog_offset_dac_val = 0x8000; // -2.5 < V < 2.5 volts: 32768 (0x8000); 0 < V < 5 volts: 65535; -5 < V < 0 volts: 0

      //  set ADC offsets (DAC)
      // some details: below in the loop, there are some numbers.  They translate to: 
      // 0x80000000 // DAC CTRL Mode: Write Command
      // 0x2000000  // DAC Command Mode: write to Input
      // 0xf00000   // DAC Address bits: ALL DACs

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Enabling the internal reference..." << std::endl;
      fail = 0;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG  ,0x88f00001);
      if(rc!=0) fail++;
      usleep(50);
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_DAC_OFFSET_CTRL_REG  ,0x88f00001);
      if(rc!=0) fail++;
      usleep(50);
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_DAC_OFFSET_CTRL_REG ,0x88f00001);
      if(rc!=0) fail++;
      usleep(50);
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_DAC_OFFSET_CTRL_REG,0x88f00001);
      if(rc!=0) fail++;
      usleep(50);
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done." << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed " << fail << " times! " << std::endl;;

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Now implementing configuration... " << std::endl;
      fail = 0;
      for(int i=0;i<4;i++){ // over all 4 ADC-FPGAs
         adc_dac_offset = i*SIS3316_FPGA_ADC_REG_OFFSET;    //                                   write cmd    ??           all DACs   ??
         rc = CommDriver::vme_write32(vme_handle,adc_dac_offset + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG,0x80000000 + 0x8000000 +  0xf00000 + 0x1); // set internal reference 
         if(rc!=0) fail++;
         usleep(50); //unsigned int uint_usec                                                     write cmd  write to input  all DACs           offset setting  
         rc = CommDriver::vme_write32(vme_handle,adc_dac_offset + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG,0x80000000 + 0x2000000 +  0xf00000 + ((analog_offset_dac_val & 0xffff) << 4) );  //
         if(rc!=0) fail++;
         usleep(50); //unsigned int uint_usec                                                     ??
         rc = CommDriver::vme_write32(vme_handle,adc_dac_offset + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG,0xC0000000 );  //
         if(rc!=0) fail++;
         usleep(50); //unsigned int uint_usec
      }
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done. "  << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed " << fail << " times! " << std::endl; 

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Setting the trigger gate window length register..." << std::endl;
      fail = 0;
      data32 = (trigger_gate_window_length - 2) & 0xffff;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG,data32);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_TRIGGER_GATE_WINDOW_LENGTH_REG,data32);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_TRIGGER_GATE_WINDOW_LENGTH_REG,data32);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_TRIGGER_GATE_WINDOW_LENGTH_REG,data32);
      if(rc!=0) fail++;
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed " << fail << " times! " << std::endl;

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Setting the pre-trigger delay value... " << std::endl;
      fail = 0;
      data32 = 0x0; // 2042; 
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG,data32);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG,data32);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG,data32);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG,data32);
      if(rc!=0) fail++;
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed " << fail << " times!" << std::endl;

      // Disable/Enable LEMO Input "TI" as External Trigger
      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Setting the trigger type... " << std::endl;
      if (auto_trigger_enable==1) {
         data32 = 0x0;  // Disable NIM Input "TI"
      }else{
         data32 = 0x10; // Enable NIM Input "TI"
      }
      rc = CommDriver::vme_write32(vme_handle,SIS3316_NIM_INPUT_CONTROL_REG,data32);
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! " << std::endl;

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Enabling external triggers... " << std::endl;
      // data32 = 0x100; // external trigger function as trigger enable   
      // data32 = 0x400; // external timestamp clear enabled  
      data32 = 0x500; // external trigger function as trigger enable + external timestamp clear enabled  
      //data32 = 0x0;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ACQUISITION_CONTROL_STATUS,data32);
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed " << fail << " times! " << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Clearing the timestamp... " << std::endl;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_KEY_TIMESTAMP_CLEAR ,0x0);
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done. " << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed " << fail << " times! " << std::endl;
      usleep(5);        // it's probably best to wait a bit before starting... 

      if(rc!=0) std::cout << "[SISInterface::initialize_3316]: Initialization failed! " << std::endl;

      return rc;
   }
   //______________________________________________________________________________
   int reinitialize(sisDigitizer_t myADC){
      int rc=-1; 
      int modID = myADC.modID; 
      if(modID==3302){
         rc = reinitialize_3302(myADC); 
      }else if(modID==3316){
         rc = reinitialize_3316(myADC);
      }else{
         std::cout << "[SISInterface::initialize]: Invalid module ID " << modID << std::endl;
         return -1;
      } 
      return rc;
   }
   //______________________________________________________________________________
   int reinitialize_3302(sisDigitizer_t myADC){
      // re-initialize the digitizer in the case that the event length has changed

      int vme_handle = myADC.vmeHandle;

      // now set the number of samples recorded before each event stops
      // the number of events is the "larger unit" compared to 
      // number of samples... that is, 1 event = N samples.  
      double signal_length   = myADC.signalLength;
      double sampling_period = myADC.clockPeriod;
      double event_length_f  = signal_length/sampling_period; // time_of_signal/sample_period = time_of_signal/(time/sample) = total number of samples per event
      u_int32_t event_length = (u_int32_t)event_length_f;
      int event_length_int   = (int)event_length_f;

      char msg[512]; 
      // sprintf(msg,"[SISInterface::reinitialize_3302]: signal_length = %.0lf, sampling period = %.3E, event_length = %.0lf",signal_length,sampling_period,event_length_f);
      // std::cout << msg << std::endl; 

      sprintf(msg,"[SISInterface::reinitialize_3302]: Setting up to record %d samples per event...",event_length_int);
      if(isDebug) std::cout << msg << std::endl;

      // set the event length 
      u_int32_t data32 = (event_length - 4) & 0xfffffC;       // what is this wizardry? no idea, from StruckADC manual.
      int rc = CommDriver::vme_write32(vme_handle,SIS3302_SAMPLE_LENGTH_ALL_ADC,data32);

      int NumberOfEvents = myADC.numberOfEvents;

      int PULSES_PER_READ = 5; 

      if(NumberOfEvents>PULSES_PER_READ){
         rc = CommDriver::vme_write32(vme_handle,SIS3302_MAX_NOF_EVENT,PULSES_PER_READ);
      }else{
         rc = CommDriver::vme_write32(vme_handle,SIS3302_MAX_NOF_EVENT,NumberOfEvents);
      }

      if(isDebug) std::cout << "[SISInterface::reinitialize_3302]: Configuration complete. " << std::endl;

      rc = CommDriver::vme_read32(vme_handle,SIS3302_KEY_ARM,0x0);

      return rc;
   } 
   //______________________________________________________________________________
   int reinitialize_3316(sisDigitizer_t myADC){
      // re-initialize the digitizer in the case that the event length has changed

      int vme_handle = myADC.vmeHandle; 

      int rc=0;
      int use_ext_raw_buf = 0;

      u_int32_t SixtyFourK=64000;
      u_int32_t read_data=0;
      // u_int32_t data32=0;
      u_int32_t raw_data_buf_reg=0;
      u_int32_t ext_raw_data_buf_reg=0;

      double sample_size_bytes    = 2.;

      // u_int32_t raw_buf_max       = 33554430;    // maximum of raw data buffer + extended raw data buffer (I think...) 
      u_int32_t raw_buf_max       = SixtyFourK;
      u_int32_t tot_buf_max       = 33554430;

      // general settings and data

      // input from user 
      u_int32_t input_nof_samples      = (u_int32_t)myADC.numberOfSamples; // number of samples  
      u_int32_t NEvents                = (u_int32_t)myADC.numberOfEvents;  // number of events 
      u_int32_t event_length           = input_nof_samples;                 // number of samples per event  

      unsigned long int NEventsOnADC   = 1;                                 // we'll print 1 event to file; so we make the address threshold hold 1 event.  

      // bookkeeping
      double input_nof_samples_mb      = ( (double)input_nof_samples )*sample_size_bytes/1E+6;

      char msg[512]; 

      if(input_nof_samples>raw_buf_max){
         use_ext_raw_buf      = 1;
         ext_raw_data_buf_reg = event_length;
         sprintf(msg,"[SISInterface::reinitialize_3316]: Using EXTENDED raw data buffer (number of samples = %lu) \n",(unsigned long)input_nof_samples);
         std::cout << msg << std::endl;
      }else{
         // ANDed with 1s to make sure it's 16 bits wide; 
         // bit-shifted to the left by 16 to meet register requirements 
         raw_data_buf_reg     = (input_nof_samples & 0xffff) << 16;
      }

      if(input_nof_samples>tot_buf_max){
         if(isDebug) std::cout << "[SISInterface::reinitialize_3316]: Number of samples too big!  Setting to maximum... " << std::endl;
         use_ext_raw_buf      = 1;
         ext_raw_data_buf_reg = tot_buf_max - 1;
      }

      unsigned long int addr_thresh        = (unsigned long int)( NEventsOnADC*event_length/2 );  // FIXME: Should be in number of 32-bit words! 
      unsigned long int max_read_nof_words = NEventsOnADC*event_length;

      std::cout << "[SISInterface::reinitialize_3316]: Initializing..." << std::endl;

      if(isDebug){
         sprintf(msg,"Event length:                        %lu (%.3lf MB) \n",(unsigned long)event_length,input_nof_samples_mb);
         std::cout << msg << std::endl;
         sprintf(msg,"Number of events:                    %d    \n",NEvents);
         std::cout << msg << std::endl;
         sprintf(msg,"Address threshold:                   %lu 32-bit words \n",addr_thresh );
         std::cout << msg << std::endl;
         sprintf(msg,"Total number of expected data words: %lu   \n",max_read_nof_words);
         std::cout << msg << std::endl;
      }

      unsigned long int data_low=0;
      unsigned long int data_high=0;
      unsigned long int sum=0;
      sum+=0;

      int fail=0;

      if(isDebug) std::cout << "[SISInterface::reinitialization_3316]: Issuing key disarm..." << std::endl;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_KEY_DISARM, 0x0);
      if(isDebug && rc==0) std::cout << "[SISInterface::reinitialization_3316]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::reinitialization_3316]: Failed!" << std::endl;
      usleep(1);

      if(use_ext_raw_buf==0){
         if(isDebug) std::cout << "[SISInterface::reinitialization_3316]: Writing data to raw data buffer config register... " << std::endl;
         fail = 0;
         rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG,raw_data_buf_reg);
         if(rc!=0) fail++;
         rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG,raw_data_buf_reg);
         if(rc!=0) fail++;
         rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG,raw_data_buf_reg);
         if(rc!=0) fail++;
         rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG,raw_data_buf_reg);
         if(rc!=0) fail++;
         if(isDebug && fail==0) std::cout << "[SISInterface::reinitialization_3316]: Done. " << std::endl;
         if(isDebug || fail!=0) std::cout << "[SISInterface::reinitialization_3316]: Failed " << fail << " times!" << std::endl;

         std::cout << "[SISInterface::reinitialization_3316]: Reading data from raw data buffer config register... " << std::endl;
         fail = 0;
         rc  = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG,&read_data);
         if(rc!=0) fail++;
         data_low  =  read_data & 0x0000ffff;                 // low bytes 
         data_high = (read_data & 0xffff0000)/pow(2,16);      // high bytes 
         sum       =  data_low + data_high;
         if(isDebug){
            sprintf(msg,"[SISInterface::reinitialization_3316]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
            std::cout << msg << std::endl;
         }
         // std::cout << "low bytes:  %lu \n",data_low);  
         // std::cout << "high bytes: %lu \n",data_high);  
         // std::cout << "sum:        %lu \n",sum);  
         rc  = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG,&read_data);
         if(rc!=0) fail++;
         data_low  =  read_data & 0x0000ffff;                 // low bytes 
         data_high = (read_data & 0xffff0000)/pow(2,16);      // high bytes 
         sum       =  data_low + data_high;
         if(isDebug){
            sprintf(msg,"[SISInterface::reinitialization_3316]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
            std::cout << msg << std::endl;
         }
         // std::cout << "low bytes:  %lu \n",data_low);  
         // std::cout << "high bytes: %lu \n",data_high);  
         // std::cout << "sum:        %lu \n",sum);  
         rc  = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG,&read_data);
         if(rc!=0) fail++;
         data_low  =  read_data & 0x0000ffff;                 // low bytes 
         data_high = (read_data & 0xffff0000)/pow(2,16);      // high bytes 
         sum       =  data_low + data_high;
         if(isDebug){
            sprintf(msg,"[SISInterface::reinitialization_3316]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
            std::cout << msg << std::endl;
         }
         // std::cout << "low bytes:  %lu \n",data_low);  
         // std::cout << "high bytes: %lu \n",data_high);  
         // std::cout << "sum:        %lu \n",sum);  
         rc  = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG,&read_data);
         if(rc!=0) fail++;
         data_low  =  read_data & 0x0000ffff;                 // low bytes 
         data_high = (read_data & 0xffff0000)/pow(2,16);      // high bytes 
         sum       =  data_low + data_high;
         if(isDebug){
            sprintf(msg,"[SISInterface::reinitialization_3316]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
         }
         // std::cout << "low bytes:  %lu \n",data_low);  
         // std::cout << "high bytes: %lu \n",data_high);  
         // std::cout << "sum:        %lu \n",sum);  
         if(isDebug && fail==0) std::cout << "[SISInterface::reinitialization_3316]: Done. " << std::endl;
         if(isDebug || fail!=0) std::cout << "[SISInterface::reinitialization_3316]: Failed " << fail << " times! " << std::endl;
      }else if(use_ext_raw_buf==1){
         if(isDebug) std::cout << "[SISInterface::reinitialization_3316]: Writing data to EXTENDED raw data buffer config register..." << std::endl; 
         fail = 0;
         rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG,ext_raw_data_buf_reg);
         if(rc!=0) fail++;
         rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG,ext_raw_data_buf_reg);
         if(rc!=0) fail++;
         rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG,ext_raw_data_buf_reg);
         if(rc!=0) fail++;
         rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG,ext_raw_data_buf_reg);
         if(rc!=0) fail++;
         if(isDebug && fail==0) std::cout << "[SISInterface::reinitialization_3316]: Done. " << std::endl;
         if(isDebug || fail!=0) std::cout << "[SISInterface::reinitialization_3316]: Failed " << fail << " times! " << std::endl;
         if(isDebug) std::cout << "[SISInterface::reinitialization_3316]: Reading data from EXTENDED raw data buffer config register... " << std::endl;
         fail = 0;
         rc  = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CH1_4_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG,&read_data);
         if(rc!=0) fail++;
         data_low  =  read_data & 0x00000fff;                 // low bytes 
         data_high = (read_data & 0x00fff000)/pow(2,12);      // high bytes 
         sum       = data_low + data_high;
         if(isDebug){
            sprintf(msg,"[SISInterface::reinitialization_3316]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
            std::cout << msg << std::endl;
         }
         // std::cout << "low bytes:  %lu \n",data_low);  
         // std::cout << "high bytes: %lu \n",data_high);  
         // std::cout << "sum:        %lu \n",sum);  
         rc  = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CH5_8_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG,&read_data);
         if(rc!=0) fail++;
         data_low  =  read_data & 0x00000fff;                 // low bytes 
         data_high = (read_data & 0x00fff000)/pow(2,12);      // high bytes 
         sum       = data_low + data_high;
         if(isDebug){
            sprintf(msg,"[SISInterface::reinitialization_3316]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
         }
         // std::cout << "low bytes:  %lu \n",data_low);  
         // std::cout << "high bytes: %lu \n",data_high);  
         // std::cout << "sum:        %lu \n",sum);  
         rc  = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CH9_12_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG,&read_data);
         if(rc!=0) fail++;
         data_low  =  read_data & 0x00000fff;                 // low bytes 
         data_high = (read_data & 0x00fff000)/pow(2,12);      // high bytes 
         sum       = data_low + data_high;
         if(isDebug){
            sprintf(msg,"[SISInterface::reinitialization_3316]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
            std::cout << msg << std::endl;
         }
         // std::cout << "low bytes:  %lu \n",data_low);  
         // std::cout << "high bytes: %lu \n",data_high);  
         // std::cout << "sum:        %lu \n",sum);  
         rc  = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CH13_16_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG,&read_data);
         if(rc!=0) fail++;
         data_low  =  read_data & 0x00000fff;                 // low bytes 
         data_high = (read_data & 0x00fff000)/pow(2,12);      // high bytes 
         sum       = data_low + data_high;
         if(isDebug){
            sprintf(msg,"[SISInterface::reinitialization_3316]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
            std::cout << msg << std::endl;
         }
         // std::cout << "low bytes:  %lu \n",data_low);  
         // std::cout << "high bytes: %lu \n",data_high);  
         // std::cout << "sum:        %lu \n",sum);  
         if(isDebug && fail==0) std::cout << "[SISInterface::reinitialization_3316]: Done. " << std::endl;
         if(isDebug || fail!=0) std::cout << "[SISInterface::reinitialization_3316]: Failed " << fail << " times! " << std::endl;

         if(rc!=0) std::cout << "[SISInterface::reinitialization_3316]: Failed! " << std::endl;
      }
      return rc;
   }
   //______________________________________________________________________________
   int configure_clock_3316(sisDigitizer_t myADC,int use_ext_clock,int adc_125MHz_flag){
      int rc = 0;
      int fail = 0;

      char msg[512]; 

      int vme_handle = myADC.vmeHandle; 

      unsigned int clock_N1div = 0;
      unsigned int clock_HSdiv = 0;
      unsigned int iob_delay_value = 0;

      u_int32_t read_data = 0;

      int ClockFreq=0;
      int ClockFreq_in_units=0;

      std::string units = myADC.clockFreqUnits; 

      if(use_ext_clock==1){
         ClockFreq          = (int)myADC.clockFrequency;
         units              = myADC.clockFreqUnits;
         ClockFreq_in_units = 0;
         if( units.compare("kHz")==0 ) ClockFreq_in_units = ClockFreq/1E+3;
         if( units.compare("MHz")==0 ) ClockFreq_in_units = ClockFreq/1E+6;
         if( units.compare("GHz")==0 ) ClockFreq_in_units = ClockFreq/1E+9;
      }

      if(use_ext_clock==0){
         if(adc_125MHz_flag==0){
            // 250  MHz
            if(isDebug) std::cout << "[SISInterface::configure_clock_3316]: Using internal clock: 250 MHz..." << std::endl;
            clock_N1div      =  4  ;
            clock_HSdiv      =  5  ;
            // iob_delay_value  =  0x48   ; // ADC FPGA version A_0250_0003
            iob_delay_value  =  0x1008 ; // ADC FPGA version A_0250_0004 and higher
         }else{
            if(isDebug) std::cout << "[SISInterface::configure_clock_3316]: Using internal clock: 125 MHz..." << std::endl;
            clock_N1div      =  8  ;
            clock_HSdiv      =  5  ;
            // iob_delay_value  =  0x7F   ; // ADC FPGA version A_0250_0003
            iob_delay_value  =  0x1020 ; // ADC FPGA version A_0250_0004 and higher
         }
         rc = change_frequency_HSdiv_N1div(vme_handle,0,clock_HSdiv,clock_N1div);
         if(isDebug && rc==0) std::cout << "[SISInterface::configure_clock_3316]: Done." << std::endl;
         if(isDebug || rc!=0) std::cout << "[SISInterface::configure_clock_3316]: Failed!" << std::endl;
      }else if(use_ext_clock==1){
         iob_delay_value = 0x0;
         if(isDebug){
            sprintf(msg,"[SISInterface::configure_clock_3316]: Using external clock: %d %s",ClockFreq_in_units,units.c_str());
            std::cout << msg << std::endl;
            sprintf(msg,"[SISInterface::configure_clock_3316]: Setting sample clock distribution control to 3 (external lemo)...\n");
            std::cout << msg << std::endl;
         }
         rc = CommDriver::vme_write32(vme_handle,SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL, 0x3);
         if(isDebug && rc==0) std::cout << "[SISInterface::configure_clock_3316]: Done. " << std::endl;
         if(isDebug || rc!=0) std::cout << "[SISInterface::configure_clock_3316]: Failed!" << std::endl;

         rc = CommDriver::vme_read32(vme_handle,SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL, &read_data);
         if(isDebug){
            sprintf(msg,"[SISInterface::configure_clock_3316]: Read sample clock distribution: 0x%08x\n", read_data);
            std::cout << msg << std::endl;
         }
         if(isDebug && rc==0) std::cout << "[SISInterface::configure_clock_3316]: Done. " << std::endl;
         if(isDebug || rc!=0) std::cout << "[SISInterface::configure_clock_3316]: Failed!" << std::endl;

         if(isDebug) std::cout << "[SISInterface::configure_clock_3316]: Bypassing precision clock multiplier..." << std::endl;
         rc = vme_write_si5325(vme_handle, 0, 0x2);
         if(isDebug && rc==0) std::cout << "[SISInterface::configure_clock_3316]: Done. " << std::endl;
         if(isDebug || rc!=0) std::cout << "[SISInterface::configure_clock_3316]: Failed! " << std::endl;

         if(isDebug) std::cout << "[SISInterface::configure_clock_3316]: Powering down the old clock..." << std::endl;
         rc = vme_write_si5325(vme_handle, 11, 0x02);
         if(isDebug && rc==0) std::cout << "[SISInterface::configure_clock_3316]: Done. " << std::endl;
         if(isDebug || rc!=0) std::cout << "[SISInterface::configure_clock_3316]: Failed! " << std::endl;
      }
      usleep(1000);

      if(isDebug) std::cout << "[SISInterface::configure_clock_3316]: Resetting the DCM/PLL of all FPGAs... " << std::endl;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_KEY_ADC_CLOCK_DCM_RESET,0x0);
      if(isDebug && rc==0) std::cout << "[SISInterface::configure_clock_3316]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::configure_clock_3316]: Failed! " << std::endl;
      usleep(10000);   // wait 10 ms for clock to be stable; should be fine after 5 ms, but we wait longer  

      fail = 0;
      if(isDebug) std::cout << "[SISInterface::configure_clock_3316]: Setting the input tap delays... " << std::endl;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG  , 0xf00 ); // Calibrate IOB _delay Logic
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG  , 0xf00 ); // Calibrate IOB _delay Logic
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG , 0xf00 ); // Calibrate IOB _delay Logic
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0xf00 ); // Calibrate IOB _delay Logic
      if(rc!=0) fail++;
      usleep(1000) ;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG  , 0x300 + iob_delay_value ); // set IOB _delay Logic
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG  , 0x300 + iob_delay_value ); // set IOB _delay Logic
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG , 0x300 + iob_delay_value ); // set IOB _delay Logic
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0x300 + iob_delay_value ); // set IOB _delay Logic
      if(rc!=0) fail++;
      usleep(1000) ;

      if(isDebug && fail==0) std::cout << "[SISInterface::configure_clock_3316]: Done. " << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::configure_clock_3316]: Failed " << fail << " times! " << std::endl;

      return rc;
   }
   //______________________________________________________________________________
   int adc_spi_setup(int vme_handle,int adc_125MHz_flag){
      // this needs to be done to turn on all the ADC outputs properly.
      // pulled from the sis3316_class.cpp file.  

      // adc_fpga_group: 0,1,2,3
      // adc_chip: 0 or 1
      //                              -1 : not all adc chips have the same chip ID
      //                              >0 : VME Error Code

      int rc;
      unsigned int adc_chip_id;
      unsigned int addr, data;

      char msg[512]; 

      // disable ADC output
      for(unsigned i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++) {
         addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((i_adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
         rc   = CommDriver::vme_write32(vme_handle,addr,0x0); //  
         if(rc != 0){
            return rc ;
         }
      }

      // dummy loop to access each adc chip one time after power up -- add 12.02.2015
      for(unsigned i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++) {
         for(unsigned i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++) {
            adc_spi_read(vme_handle,i_adc_fpga_group,i_adc_chip,1,&data);
         }
      }

      // reset 
      for(unsigned i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++) {
         for(unsigned i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++) {
            rc = adc_spi_write(vme_handle,i_adc_fpga_group,i_adc_chip,0x0,0x24); // soft reset
         }
         usleep(10) ; // after reset
      }

      rc = adc_spi_read(vme_handle,0,0,1,&adc_chip_id); // read chip Id from adc chips ch1/2

      for(unsigned i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++) {
         for(unsigned i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++) {
            adc_spi_read(vme_handle,i_adc_fpga_group, i_adc_chip,1,&data);
            // sprintf(msg,"i_adc_fpga_group = %d   i_adc_chip = %d    data = 0x%08x     adc_chip_id = 0x%08x     \n", i_adc_fpga_group, i_adc_chip, data, adc_chip_id);
            // std::cout << msg << std::endl;
            if (data != adc_chip_id) {
               sprintf(msg,"i_adc_fpga_group = %d i_adc_chip = %d data = 0x%08x adc_chip_id = 0x%08x",
                       i_adc_fpga_group, i_adc_chip, data, adc_chip_id);
               std::cout << msg << std::endl;
               return -1 ;
            }
         }
      }

      // adc_125MHz_flag = 0;
      if ((adc_chip_id&0xff)==0x32){
         adc_125MHz_flag = 1;
      }

      // reg 0x14 : Output mode
      if (adc_125MHz_flag==0) { // 250 MHz chip AD9643
         data = 0x04 ;   //  Output inverted (bit2 = 1)
      }else{
         // 125 MHz chip AD9268
         data = 0x40 ;   // Output type LVDS (bit6 = 1), Output inverted (bit2 = 0) !
      }

      for(unsigned i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++) {
         for(unsigned i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++) {
            adc_spi_write(vme_handle,i_adc_fpga_group,i_adc_chip,0x14,data);
         }
      }

      // reg 0x18 : Reference Voltage / Input Span
      if(adc_125MHz_flag==0){
         // 250 MHz chip AD9643
         data = 0x0 ;    //  1.75V
      }else{
         // 125 MHz chip AD9268
         //data = 0x8 ;  //  1.75V
         data = 0xC0 ;   //  2.0V
      }

      for(unsigned i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++) {
         for(unsigned i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++) {
            adc_spi_write(vme_handle,i_adc_fpga_group,i_adc_chip,0x18,data);
         }
      }

      // reg 0xff : register update
      data = 0x01 ;   // update
      for(unsigned i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++) {
         for(unsigned i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++) {
            adc_spi_write(vme_handle,i_adc_fpga_group,i_adc_chip,0xff, data);
         }
      }

      // enable ADC output
      for(unsigned i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++) {
         addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((i_adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
         rc = CommDriver::vme_write32(vme_handle,addr,0x1000000); //  set bit 24
         if (rc != 0) {return rc ; }
      }

      return 0; 
   }
   //______________________________________________________________________________
   int adc_spi_write(int vme_handle,unsigned int adc_fpga_group,unsigned int adc_chip,unsigned int spi_addr,unsigned int spi_data){
      // this needs to be done to turn on all the ADC outputs properly.
      // pulled from the sis3316_class.cpp file. 

      volatile int rc;  // huh?? 
      unsigned int data;
      unsigned int addr;
      unsigned int uint_adc_mux_select;
      unsigned int pollcounter;
      pollcounter = 1000;

      if(adc_fpga_group > 4){
         return -1;
      }

      if(adc_chip > 2){
         return -1;
      }

      if(spi_addr > 0xffff){
         return -1;
      }

      if(adc_chip == 0) {
         uint_adc_mux_select = 0 ;       // adc chip ch1/ch2     
      }else {
         uint_adc_mux_select = 0x400000 ; // adc chip ch3/ch4            
      }

      // read register to get the information of bit 24 (adc output enabled)
      addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
      rc = CommDriver::vme_read32(vme_handle,addr,&data); //  

      if(rc!=0){
         return rc; 
      }

      data = data & 0x01000000 ; // save bit 24
      data = data + 0x80000000 + uint_adc_mux_select + ((spi_addr & 0xffff) << 8) + (spi_data & 0xff) ;
      addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
      rc   = CommDriver::vme_write32(vme_handle,addr,data);
      //usleep(1000); 

      addr = SIS3316_ADC_FPGA_SPI_BUSY_STATUS_REG ;

      // char msg[512]; 
      // sprintf(msg,"poll_on_spi_busy: pollcounter = 0x%08x    \n", pollcounter );
      // std::cout << msg << std::endl;

      do{
         rc = CommDriver::vme_read32(vme_handle,addr,&data); 
         pollcounter--; 
      }while( ((data & 0x0000000f)!=0x00000000)&&(pollcounter>0)&&(rc==0) ); // changed 2.12.2014. VME FPGA verison 0x0005 and lower
      //  printf("poll_on_spi_busy: pollcounter = 0x%08x   data = 0x%08x   rc = 0x%08x \n", pollcounter, data, rc);

      if(rc!=0){
         return rc; 
      }
      if(pollcounter==0){
         return -2; 
      }
      return 0;
   }
   //______________________________________________________________________________
   int adc_spi_read(int vme_handle,unsigned int adc_fpga_group,unsigned int adc_chip,unsigned int spi_addr,unsigned int *spi_data){
      // this needs to be done to turn on all the ADC outputs properly.
      // pulled from the sis3316_class.cpp file. 

      char msg[512]; 
      int rc;
      unsigned int data ;
      unsigned int addr ;
      unsigned int uint_adc_mux_select;
      unsigned int pollcounter;

      pollcounter = 1000;

      if (adc_fpga_group > 4) {return -1;}
      if (adc_chip > 2) {return -1;}
      if (spi_addr > 0x1fff) {return -1;}

      if (adc_chip == 0) {
         uint_adc_mux_select = 0 ;       // adc chip ch1/ch2     
      }else{
         uint_adc_mux_select = 0x400000 ; // adc chip ch3/ch4            
      }

      // read register to get the information of bit 24 (adc output enabled)
      addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
      rc = CommDriver::vme_read32(vme_handle,addr,&data); //  

      if(isDebug){
         if (rc!=0){
            sprintf(msg,"adc_spi_read vme_A32D32_read 1: rc = 0x%08x", rc);
            std::cout << msg << std::endl;
            return rc;
         }
      }

      data = data & 0x01000000 ; // save bit 24
      data = data + 0xC0000000 + uint_adc_mux_select + ((spi_addr & 0x1fff) << 8)  ;

      addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
      rc = CommDriver::vme_write32(vme_handle,addr,data); //

      if(isDebug){
         if (rc!=0) {
            sprintf(msg,"adc_spi_read vme_A32D32_write 1: rc = 0x%08x", rc);
            std::cout << msg << std::endl;
         }
      }

      addr = SIS3316_ADC_FPGA_SPI_BUSY_STATUS_REG ;

      do{ // the logic is appr. 20us busy 
         rc = CommDriver::vme_read32(vme_handle,addr,&data); //  
         if(isDebug){
            if (rc != 0) {
               sprintf(msg,"adc_spi_read vme_A32D32_read 2: rc = 0x%08x", rc);
               std::cout << msg << std::endl;
            }
         }
         pollcounter--;
         //} while (((data & 0x80000000) == 0x80000000) && (pollcounter > 0) && (rc == 0)); // VME FPGA Version 0x0006 and higher
   } while (((data & 0x0000000f) != 0x00000000) && (pollcounter > 0) && (rc == 0)); // changed 2.12.2014,  VME FPGA Version 0x0005 and lower

      if(isDebug){
         sprintf(msg,"adc_spi_read pollcounter: pollcounter = 0x%08x     \n", pollcounter);
         std::cout << msg << std::endl;
      }

      if (rc != 0) {return rc ; }
      if (pollcounter == 0) {
         return -2 ;
      }

      usleep(20) ; //

      //addr = SIS3316_ADC_CH1_4_SPI_READBACK_REG  ; // removed 21.01.2015
      addr = SIS3316_ADC_CH1_4_SPI_READBACK_REG +  ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ; // changed 21.01.2015

      rc = CommDriver::vme_read32(vme_handle,addr,&data); //  

      if(isDebug){
         if(rc != 0){
            sprintf(msg,"adc_spi_read vme_A32D32_read 3: rc = 0x%08x     \n", rc);
            std::cout << msg << std::endl;
         }
      }

      if (rc != 0) {return rc ; }

      *spi_data = data & 0xff ;

      return 0 ;
}
//_____________________________________________________________________________
int vme_write_si5325(int vme_handle, u_int32_t si5325_addr, u_int32_t data32){

   int rc, poll_counter;
   u_int32_t write_data, read_data;
   int POLL_COUNTER_MAX = 100;

   // tell SI5325 what address we want to write to
   write_data = 0x0000 + (si5325_addr & 0xff);
   rc = CommDriver::vme_write32(vme_handle, SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, write_data);
   if (rc != 0) { return -1; }
   usleep(10000);

   // confirm that the SI5325 understood/processed our request to set the write address
   poll_counter = 0;
   do{
      poll_counter++;
      CommDriver::vme_read32(vme_handle, SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, &read_data);
   }while ( ( (read_data & 0x80000000) == 0x80000000) && (poll_counter < POLL_COUNTER_MAX) );
   if (poll_counter == POLL_COUNTER_MAX) {      return -2 ;     }

   // actually write data to the specificed SI5325 address
   write_data = 0x4000 + (data32 & 0xff);
   rc = CommDriver::vme_write32(vme_handle, SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, write_data);
   if (rc != 0) { return -1; }
   usleep(10000);

   // again confirm that the SI5325 understood/processed our requested write
   poll_counter = 0;
   do{
      poll_counter++;
      CommDriver::vme_read32(vme_handle, SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, &read_data);
   }while ( ( (read_data & 0x80000000) == 0x80000000) && (poll_counter < POLL_COUNTER_MAX) );
   if (poll_counter == POLL_COUNTER_MAX) {      return -2 ;     }

   return 0;
}
//_____________________________________________________________________________
int change_frequency_HSdiv_N1div(int vme_handle,int osc, unsigned hs_div_val, unsigned n1_div_val){

   // change_frequency_HSdiv_N1div                             
   // hs_div_val: allowed values are [4, 5, 6, 7, 9, 11]       
   // n1_div_val: allowed values are [2, 4, 6, .... 124, 126]  
   // Sample Frequence =  5 GHz / (hs_div_val * n1_div_val)    
   //                                                          
   // example:                                                 
   // hs_div_val = 5                                           
   // n1_div_val = 4                                           
   // Sample Frequence =  5 GHz / 20 = 250 MHz                 

   int rc;
   unsigned i ;
   unsigned N1div ;
   unsigned HSdiv ;
   unsigned HSdiv_reg[6] ;
   unsigned HSdiv_val[6] ;
   unsigned char freqSI570_high_speed_rd_value[6];
   unsigned char freqSI570_high_speed_wr_value[6];

   if(osc > 3 || osc < 0){
      return -100;
   }

   HSdiv_reg[0] =  0 ;
   HSdiv_val[0] =  4 ;

   HSdiv_reg[1] =  1 ;
   HSdiv_val[1] =  5 ;

   HSdiv_reg[2] =  2 ;
   HSdiv_val[2] =  6 ;

   HSdiv_reg[3] =  3 ;
   HSdiv_val[3] =  7 ;

   HSdiv_reg[4] =  5 ;
   HSdiv_val[4] =  9 ;

   HSdiv_reg[5] =  7 ;
   HSdiv_val[5] =  11 ;

   HSdiv = 0xff ;
   for (i=0;i<6;i++){
      if (HSdiv_val[i] == hs_div_val) {
         HSdiv = HSdiv_reg[i] ;
      }
   }
   if (HSdiv > 11) {
      return -101;
   }
   // gtr than 127 or odd then return
   if((n1_div_val > 127) || ((n1_div_val & 0x1) == 1) || (n1_div_val == 0) ) {
      return -102;
   }
   N1div = n1_div_val - 1 ;

   rc = Si570ReadDivider(vme_handle,osc, freqSI570_high_speed_rd_value);
   if(rc){
      printf("Si570ReadDivider = %d \n",rc);
      return rc;
   }
   freqSI570_high_speed_wr_value[0] = ((HSdiv & 0x7) << 5) + ((N1div & 0x7c) >> 2);
   freqSI570_high_speed_wr_value[1] = ((N1div & 0x3) << 6) + (freqSI570_high_speed_rd_value[1] & 0x3F);
   freqSI570_high_speed_wr_value[2] = freqSI570_high_speed_rd_value[2];
   freqSI570_high_speed_wr_value[3] = freqSI570_high_speed_rd_value[3];
   freqSI570_high_speed_wr_value[4] = freqSI570_high_speed_rd_value[4];
   freqSI570_high_speed_wr_value[5] = freqSI570_high_speed_rd_value[5];

   rc = set_frequency(vme_handle,osc,freqSI570_high_speed_wr_value);
   if(rc){
      printf("set_frequency = %d \n",rc);
      return rc;
   }

   return 0;
}
//______________________________________________________________________________
int set_frequency(int vme_handle,int osc, unsigned char *values){

   int rc;

   if(values == NULL){
      return -100;
   }

   if(osc > 3 || osc < 0){
      return -100;
   }

   rc = Si570FreezeDCO(vme_handle,osc);
   if(rc){
      return rc;
   }

   rc = Si570Divider(vme_handle,osc, values);
   if(rc){
      return rc;
   }

   rc = Si570UnfreezeDCO(vme_handle,osc);
   if(rc){
      return rc;
   }

   rc = Si570NewFreq(vme_handle,osc);
   if(rc){
      return rc;
   }

   // min. 10ms wait
   usleep(15000); // 15 ms

   // DCM Reset  
   rc = CommDriver::vme_write32(vme_handle,0x438,0);

   if(rc){
      return rc;
   }

   // DCM Reset -> the DCM/PLL of the ADC-FPGAs will be stable after max. 5ms
   //              or check the DCM OK bits (ADC FPGA Status registers bit 20)
   usleep(5000); // 5 ms

   return 0;
}
//______________________________________________________________________________
int Si570ReadDivider(int vme_handle,int osc, unsigned char *data){

   int rc;
   char ack;
   int i;

   // start
   rc = I2cStart(vme_handle,osc);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   // address
   rc = I2cWriteByte(vme_handle,osc, OSC_ADR<<1, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   // register offset
   rc = I2cWriteByte(vme_handle,osc, 0x0D, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   rc = I2cStart(vme_handle,osc);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   // address + 1
   rc = I2cWriteByte(vme_handle,osc, (OSC_ADR<<1) + 1, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   // read data
   for(i = 0;i < 6;i++){
      ack = 1 ;
      if (i==5){
         ack = 0;
      }
      rc = I2cReadByte(vme_handle,osc, &data[i], ack);
      if(rc){
         I2cStop(vme_handle,osc);
         return rc;
      }
   }

   // stop
   rc = I2cStop(vme_handle,osc);
   if(rc){
      return rc;
   }

   return 0;

}
//______________________________________________________________________________
int Si570Divider(int vme_handle,int osc, unsigned char *data){

   int rc;
   char ack;
   int i;

   // start
   rc = I2cStart(vme_handle,osc);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   // address
   rc = I2cWriteByte(vme_handle,osc, OSC_ADR<<1, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   // register offset
   rc = I2cWriteByte(vme_handle,osc, 0x0D, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   // write data
   for(i = 0;i < 2;i++){
      rc = I2cWriteByte(vme_handle,osc, data[i], &ack);
      if(rc){
         I2cStop(vme_handle,osc);
         return rc;
      }
      if(!ack){
         I2cStop(vme_handle,osc);
         return -101;
      }
   }
 
  // stop
   rc = I2cStop(vme_handle,osc);
   if(rc){
      return rc;
   }

   return 0;

}
//______________________________________________________________________________
int Si570UnfreezeDCO(int vme_handle,int osc){

   int rc;
   char ack;

   // start
   rc = I2cStart(vme_handle,osc);

   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   // address
   rc = I2cWriteByte(vme_handle,osc, OSC_ADR<<1, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   // register offset
   rc = I2cWriteByte(vme_handle,osc, 0x89, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   // write data
   rc = I2cWriteByte(vme_handle,osc, 0x00, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   // stop
   rc = I2cStop(vme_handle,osc);
   if(rc){
      return rc;
   }

   return 0;

}
//______________________________________________________________________________
int Si570NewFreq(int vme_handle,int osc){

   int rc;
   char ack;

   // start
   rc = I2cStart(vme_handle,osc);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   // address
   rc = I2cWriteByte(vme_handle,osc,OSC_ADR<<1, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;

   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   // register offset
   rc = I2cWriteByte(vme_handle,osc, 0x87, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;

   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   // write data
   rc = I2cWriteByte(vme_handle,osc, 0x40, &ack);
   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   if(!ack){
      I2cStop(vme_handle,osc);
      return -101;
   }

   // stop
   rc = I2cStop(vme_handle,osc);
   if(rc){
      return rc;
   }

   return 0;
}
//______________________________________________________________________________
int I2cStart(int vme_handle,int osc){

   int rc;
   int i;
   unsigned int tmp;

   if(osc > 3){
      return -101;
   }

   // start
   rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), 1<<I2C_START);

   if(rc){
      return rc;
   }

   i = 0;

   do{
      // poll i2c fsm busy
      rc = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), &tmp);
      if(rc){
         return rc;
      }
      i++;
   }while((tmp & (1<<I2C_BUSY)) && (i < 1000));

   // register access problem
   if(i == 1000){
      return -100;
   }

   return 0;

}
int I2cStop(int vme_handle,int osc){

   int rc;
   int i;
   unsigned int tmp;

   if(osc > 3){
      return -101;
   }

   // stop
   rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), 1<<I2C_STOP);
   if(rc){
      return rc;
   }

   i = 0;
   do{
      // poll i2c fsm busy
      rc = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), &tmp);
      if(rc){
         return rc;
      }
      i++;
   }while((tmp & (1<<I2C_BUSY)) && (i < 1000));

   // register access problem
   if(i == 1000){
      return -100;
   }

   return 0;

}
//______________________________________________________________________________
int I2cWriteByte(int vme_handle,int osc, unsigned char data, char *ack){

   int rc;
   int i;
   unsigned int tmp;

   if(osc > 3){
      return -101;
   }

   // write byte, receive ack
   rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), 1<<I2C_WRITE ^ data);
   if(rc){
      return rc;
   }

   i = 0;
   do{
      // poll i2c fsm busy
      rc = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), &tmp);
      if(rc){
         return rc;
      }
      i++;
   }while((tmp & (1<<I2C_BUSY)) && (i < 1000));

   // register access problem
   if(i == 1000){
      return -100;
   }

   // return ack value?
   if(ack){
      // yup
      *ack = tmp & 1<<I2C_ACK ? 1 : 0;
   }

   return 0;

}
//______________________________________________________________________________
int I2cReadByte(int vme_handle,int osc, unsigned char *data, char ack){

   int rc;
   int i;
   unsigned int tmp;
   unsigned char char_tmp;

   if(osc > 3){
      return -101;
   }

   // read byte, put ack
   tmp = 1<<I2C_READ;
   tmp |= ack ? 1<<I2C_ACK : 0;
   rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), tmp);
   if(rc){
      return rc;
   }

   i = 0;
   do{
      // poll i2c fsm busy
      rc = CommDriver::vme_read32(vme_handle,SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), &tmp);
      if(rc){
         return rc;
      }
      i++;
   }while((tmp & (1<<I2C_BUSY)) && (i < 1000));

   // register access problem
   if(i == 1000){
      return -100;
   }

   char_tmp = (unsigned char) (tmp & 0xff) ;

   *data = char_tmp ;

   return 0;

}
//_____________________________________________________________________________
int call_vme_A32MBLT64FIFO_read(int vme_handle, u_int32_t vme_adr, u_int32_t* vme_data,
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords){

   // pulled from sis1100linux_vme_class.cpp
   // inputs: 
   // - vme_handle        = VME handle  
   // - vme_adr           = VME address (including base address) 
   // - vme_data          = data we want to pull from ADC 
   // - req_num_of_lwords = requested number of data words 
   // - got_num_of_lwords = obtained number of data words 
   // output: 
   // - returns error code (0 if successful)  

   struct sis1100_vme_block_req block_req;
   block_req.num=req_num_of_lwords   ; /*  */
   block_req.fifo=1;
   block_req.size=4;
   block_req.am=0x8;
   block_req.addr=vme_adr ;
   block_req.data = (u_int8_t*)vme_data ;
   if (ioctl(vme_handle, SIS3100_VME_BLOCK_READ, &block_req)<0 )  return -1 ;   /* NEW */
   *got_num_of_lwords = block_req.num;
   return block_req.error;            /* NEW */
}
//_____________________________________________________________________________
int read_DMA_Channel_PreviousBankDataBuffer(int vme_handle,                    /* VME handle */
                                            unsigned int bank2_read_flag,      /* which bank? 0 => 1, 1=>2 */
                                            unsigned int channel_no,           /* 0 to 15 */
                                            unsigned int max_read_nof_words,   /* max number of words read */
                                            unsigned int *dma_got_nof_words,   /* obtained number of words */
                                            u_int32_t *uint_adc_buffer,        /* output data? */
                                            u_int32_t NumOfSamples             /* number of samples */
                                            ){

   // taken from SIS3316 DVD
   // seems to be the code to read the data out from the ADC... 

   char msg[512]; 

   int rc;
   unsigned int data;
   unsigned int addr;
   unsigned int previous_bank_addr_value;
   unsigned int req_nof_32bit_words;
   unsigned int got_nof_32bit_words;
   unsigned int memory_bank_offset_addr;
   unsigned int max_poll_counter;

   // read previous Bank sample address
   // poll until it is valid.
   addr = SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG + ((channel_no & 0x3) * 4) + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
   max_poll_counter     = 10000;
   *dma_got_nof_words = 0;
   do {
      // printf("%d \n",__LINE__); 
      // get the previous (memory) bank address 
      rc = CommDriver::vme_read32(vme_handle,addr,&previous_bank_addr_value); //  
      if(rc!=0) {
         sprintf(msg,"[CommDriver::vme_read32]: Error: vme_A32D32_read: rc = 0x%08x   addr = 0x%08x", rc,SIS3316_MOD_BASE+addr);
         std::cout << msg << std::endl;
         return rc;
      }

      max_poll_counter--;

      if (max_poll_counter == 0) {
         sprintf(msg,"[SIS3316_um]: Error: max_poll_counter = 0x%08x ", max_poll_counter);
         std::cout << msg << std::endl;
         sprintf(msg,"[SIS3316_um]: previous bank address value: 0x%08x",previous_bank_addr_value);
         std::cout << msg << std::endl;
         return 0x900;
      }

   }while( ( (previous_bank_addr_value & 0x1000000) >> 24 )  != bank2_read_flag ); // previous Bank sample address is valid if bit 24 is equal bank2_read_flag 

   if(isDebug) sprintf(msg,"[SIS3316_um]: previous bank address value: 0x%08x \n",previous_bank_addr_value);
   std::cout << msg << std::endl;
   if(isDebug) sprintf(msg,"[SIS3316_um]: addr = 0x%08x previous bank address value = 0x%08x \n",SIS3316_MOD_BASE+addr,previous_bank_addr_value);
   std::cout << msg << std::endl;

   // check the obtained previous bank address; return zero if
   // we have nothing. 
   if( (previous_bank_addr_value & 0xffffff)==0 ){ // no data sampled !
      std::cout << "[SIS3316_um]: No data sampled!" << std::endl;
      sprintf(msg,"[SIS3316_um]: addr = 0x%08x previous bank address value = 0x%08x \n",SIS3316_MOD_BASE+addr,previous_bank_addr_value);
      std::cout << msg << std::endl;
      *dma_got_nof_words = 0;
      return 1;
   }
   // check the obtained previous bank address; return zero if
   // start Readout FSM
   // determine the memory bank offset address 
   if (bank2_read_flag == 1){
      memory_bank_offset_addr = 0x01000000; // Bank2 offset
   }else{
      memory_bank_offset_addr = 0x00000000; // Bank1 offset
   }
   // check the obtained previous bank address; return zero if
   // now locate the desired channel number 
   if( (channel_no & 0x1) != 0x1 ){    // 0,1         
      memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
   }else{
      memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
   }
   if( (channel_no & 0x2) != 0x2 ){ // 0,2         
      memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
   }else{
      memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 ..... 
   }
   // check the obtained previous bank address; return zero if

   // in case of using ETHERNET_UDP_INTERFACE it is possible to lose packets 
   // (observed with WIN7 using a "company-net", not observed with WIN7 and direct connection and not observed with LINUX)

   unsigned int retry_counter = 0 ;
   unsigned int retry_flag = 0 ;

   // data transfer control register read/write: my estimation (from page 109 of the manual) 
   // - bits 0--27:  Memory 32-bit start addresses (only have 28 bits to work with, possibly number of 32-bit words?)  
   // - bits 28--29: "space select bits" 
   // - bits 30--31: "command bits"
   // - space select bit table 
   //   bit 1    bit 0    hex            function 
   //   -------------------------------------------------------------------------
   //     0       0       0x0            Memory 1 (ch. 1 and 2) 
   //     0       1       0x40000000     Memory 2 (ch. 3 and 4) 
   //     1       0       0x80000000     Reserved  
   //     1       1       0xC0000000     Statistic counter (128 32-bit words)  
   //   -------------------------------------------------------------------------
   // - command bit table 
   //   bit 1    bit 0    hex            function 
   //   -------------------------------------------------------------------------
   //     0       -       0x0            Reset transfer FSM
   //     1       0       0x80000000     Start read transfer
   //     1       1       0xC0000000     Start write transfer        
   //   -------------------------------------------------------------------------

   // check the obtained previous bank address; return zero if
   u_int32_t START_READ_TRANSFER  = 0x80000000;
   // u_int32_t START_WRITE_TRANSFER = 0xC0000000;

      do{
      addr        = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + ( ( (channel_no >> 2) & 0x3) * 4 );
      // data        = 0x80000000 + memory_bank_offset_addr;   
      data        = START_READ_TRANSFER + memory_bank_offset_addr;
      // data = start read transfer (0x80000000) of the given address of channel no.  
      rc = CommDriver::vme_write32(vme_handle,addr,data);
      // rc = i->vme_A32D32_write(this->baseaddress + addr, data);

      if(rc != 0) {
         // printf("Error: vme_A32D32_write: rc = 0x%08x   addr = 0x%08x  data = 0x%08x \n", rc,  this->baseaddress + addr, data);
         // return rc;
      }else{
         // readout 
         addr                = SIS3316_MOD_BASE + SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3 )* SIS3316_FPGA_ADC_MEM_OFFSET);
         req_nof_32bit_words = previous_bank_addr_value & 0xffffff ;  // get the lowest 24 bits 
         // printf("number of 32-bit words: %d (0x%08x) \n",req_nof_32bit_words,req_nof_32bit_words); 

         if(req_nof_32bit_words > max_read_nof_words){
            sprintf(msg,"[SIS3316_um]: Error: exceeded the allowed number of data words! Setting to maximum = %u.",max_read_nof_words);
            std::cout << msg << std::endl;
            req_nof_32bit_words = max_read_nof_words;
         }
         usleep(5); // wait 5 us before reading out data 
         // FIFORead(vme_handle,addr,NumOfSamples);  
         // FIFO: first in, first out 
         rc = call_vme_A32MBLT64FIFO_read(vme_handle,addr,uint_adc_buffer,((req_nof_32bit_words + 1) & 0xfffffE),&got_nof_32bit_words); // N * 8-byte length  !!! 

         if(rc != 0){
            sprintf(msg,"[SIS3316_um]: Error: vme_A32MBLT64FIFO_read: rc = 0x%08x   addr = 0x%08x  req_nof_32bit_words = 0x%08x",
                    rc,SIS3316_MOD_BASE + addr,req_nof_32bit_words);
            std::cout << msg << std::endl;
            return rc;
         }

         // *dma_got_nof_words = req_nof_32bit_words;
         *dma_got_nof_words = got_nof_32bit_words;
      }

      retry_flag = 0 ;

      if(rc != 0) {
         retry_counter++ ;
         retry_flag = 0 ;
         if (retry_counter < 16) {
            retry_flag = 1 ;
         }
         if (retry_counter > 1) {
            sprintf(msg,"[SIS3316_um]: Info: retry_counter = %d",retry_counter);
         }
      }

      // reset FSM again
      addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4) ;
      data = 0x00000000; // Reset
      rc = CommDriver::vme_write32(vme_handle,addr,data);
      // rc = i->vme_A32D32_write(this->baseaddress + addr, data);

      // if(rc != 0) {
      //    sprintf("Error: vme_A32D32_write: rc = 0x%08x   addr = 0x%08x  data = 0x%08x",rc,  this->baseaddress + addr, data);
      //    std::cout << msg << std::endl;
      //    return rc;
      // }
   }while(retry_flag==1);

   if (retry_counter > 15) {
      sprintf(msg,"[SIS3316_um]: Tried too many times.");
      std::cout << msg << std::endl;
      return -1 ;
   }
   return 0 ;
}

} // ::SISInterface
