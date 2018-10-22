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
      int rc = CommDriver::vme_read32(handle,addr,data);
      modID  = data32 >> 16;
      majRev = (data32 >> 8) & 0xff;  
      minRev = data32 & 0xff;  
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

      int modId=0,majRev=0,minRev=0;
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

      int TestVal = 3;   // test code (used for test mode) 

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
         sprintf("[SISInterface::initialize_3316]: Trigger gate window length: %u (%.4E sec) \n",
                 trigger_gate_window_length,trigger_gate_window_sec);
         std::cout << msg << std::endl;
      }

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Reading the MOD ID..." << std::endl;
      int modID=0,majRev=0,minRev=0;
      int rc = get_module_id(vme_handle,SIS3316_MODID,modID,majRev,minRev);
      if( isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done."   << std::endl;
      if( isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed!" << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Issuing key reset...\n" std::endl;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_KEY_RESET, 0x0);
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. \n" std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! \n" << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Issuing key disarm...\n" << std::endl;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_KEY_DISARM, 0x0);
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. \n" << std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! \n" << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Configuring the ADC via SPI... \n" << std::endl;
      rc = adc_spi_setup(vme_handle,adc_125MHz_flag);
      if( isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. \n" << std::endl;
      if( isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! \n" << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Configuring the clock... \n" << std::endl;
      rc = SIS3316ConfigureClock(vme_handle,myADC,use_ext_clock,adc_125MHz_flag);
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. \n" << std::endl;
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! \n" << std::endl;

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Turning on the ADC chip outputs... \n");
      u_int32_t an_offset=0;
      int fail=0;
      for(i=0;i<4;i++){
         an_offset = i*SIS3316_FPGA_ADC_REG_OFFSET;
         rc  = CommDriver::vme_write32(vme_handle,an_offset + SIS3316_ADC_CH1_4_SPI_CTRL_REG, 0x01000000 ); // enable ADC chip outputs
         if(rc!=0) fail++;
      }
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done." << std::endl;
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed " << fail << " times!" << std::endl;
      usleep(1);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Setting the LEMO output 'CO'... \n");
      data32 = 0x1 ; // Select Sample Clock
      rc = CommDriver::vme_write32(vme_handle,SIS3316_LEMO_OUT_CO_SELECT_REG, data32 ); //
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. \n");
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! \n");

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Enabling the LEMO output 'TO'... \n");
      data32 = 0xffff ; // Select all triggers
      rc = CommDriver::vme_write32(vme_handle,SIS3316_LEMO_OUT_TO_SELECT_REG, data32 ); //
      if(isDebug && rc==0) std::cout << "[SISInterface::initialize_3316]: Done. \n");
      if(isDebug || rc!=0) std::cout << "[SISInterface::initialize_3316]: Failed! \n");

      // header writes 
      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Setting up the headers... \n");
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
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done. \n");
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed %d times! \n",fail);

      // gain/termination 
      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Setting the gain and termination options... \n");
      fail = 0;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_ANALOG_CTRL_REG  ,analog_ctrl_val);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_ANALOG_CTRL_REG  ,analog_ctrl_val);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_ANALOG_CTRL_REG ,analog_ctrl_val);
      if(rc!=0) fail++;
      rc = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_ANALOG_CTRL_REG,analog_ctrl_val);
      if(rc!=0) fail++;
      if(isDebug && fail==0) std::cout << "[SISInterface::initialize_3316]: Done. \n");
      if(isDebug || fail!=0) std::cout << "[SISInterface::initialize_3316]: Failed %d times! \n",fail);

      if(isDebug) std::cout << "[SISInterface::initialize_3316]: Intializing the ADC (DAC) offsets... \n");
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

      if(isDebug || gIsTest==TestVal) std::cout << "[SISInterface::initialize_3316]: Now implementing configuration... " << std::endl;
      fail = 0;
      for (i=0;i<4;i++){ // over all 4 ADC-FPGAs
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
   int reinitialize_3302(sisDigitizer_t myADC){
      // re-initialize the digitizer in the case that the event length has changed

      int vme_handle = myADC.vmeHandle;

      // now set the number of samples recorded before each event stops
      // the number of events is the "larger unit" compared to 
      // number of samples... that is, 1 event = N samples.  
      double signal_length   = myADC->fSignalLength;
      double sampling_period = myADC->fClockPeriod;
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
      int rc = CommDriver::vme_read32(vme_handle,SIS3302_SAMPLE_LENGTH_ALL_ADC,data32);

      int NumberOfEvents = myADC.numberOfEvents;

      if(NumberOfEvents>PULSES_PER_READ){
         rc = CommDriver::vme_read32(vme_handle,SIS3302_MAX_NOF_EVENT,PULSES_PER_READ);
      }else{
         rc = CommDriver::vme_read32(vme_handle,SIS3302_MAX_NOF_EVENT,NumberOfEvents);
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
      u_int32_t data32=0;
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
         sprintf("[SISInterface::reinitialize_3316]: Using EXTENDED raw data buffer (number of samples = %lu) \n",(unsigned long)input_nof_samples);
         std::cout << msg << std::endl;
      }else{
         // ANDed with 1s to make sure it's 16 bits wide; 
         // bit-shifted to the left by 16 to meet register requirements 
         raw_data_buf_reg     = (input_nof_samples & 0xffff) << 16;
      }

      if(input_nof_samples>tot_buf_max){
         if(isDebug) std::cout << "[SISInterface::reinitialize_3316]: Number of samples too big!  Setting to maximum... \n");
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
         return rc;
      }
   }


} // ::SISInterface
