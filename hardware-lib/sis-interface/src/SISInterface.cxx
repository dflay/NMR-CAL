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

      if(isDebug) printf("[SIS3316_um]: Turning on the ADC chip outputs... \n");
      u_int32_t an_offset=0;
      int fail=0;
      for(i=0;i<4;i++){
         an_offset = i*SIS3316_FPGA_ADC_REG_OFFSET;
         ret_code  = CommDriver::vme_write32(vme_handle,an_offset + SIS3316_ADC_CH1_4_SPI_CTRL_REG, 0x01000000 ); // enable ADC chip outputs
         if(ret_code!=0) fail++;
      }
      if(isDebug && fail==0) std::cout << "[SIS3316_um]: Done." << std::endl;
      if(isDebug || fail!=0) std::cout << "[SIS3316_um]: Failed " << fail << " times!" << std::endl;
      usleep(1);

      if(isDebug) printf("[SIS3316_um]: Setting the LEMO output 'CO'... \n");
      data32 = 0x1 ; // Select Sample Clock
      ret_code = CommDriver::vme_write32(vme_handle,SIS3316_LEMO_OUT_CO_SELECT_REG, data32 ); //
      if(isDebug && ret_code==0) printf("[SIS3316_um]: Done. \n");
      if(isDebug || ret_code!=0) printf("[SIS3316_um]: Failed! \n");

      if(isDebug) printf("[SIS3316_um]: Enabling the LEMO output 'TO'... \n");
      data32 = 0xffff ; // Select all triggers
      ret_code = CommDriver::vme_write32(vme_handle,SIS3316_LEMO_OUT_TO_SELECT_REG, data32 ); //
      if(isDebug && ret_code==0) printf("[SIS3316_um]: Done. \n");
      if(isDebug || ret_code!=0) printf("[SIS3316_um]: Failed! \n");

      // header writes 
      if(isDebug) printf("[SIS3316_um]: Setting up the headers... \n");
      fail = 0;
      data32 = 0x0 ;
      ret_code = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG  , data32 ); //
      if(ret_code!=0) fail++;
      data32 = 0x00400000;
      ret_code = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG  , data32 ); //
      if(ret_code!=0) fail++;
      data32 = 0x00800000 ;
      ret_code = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG , data32 ); //
      if(ret_code!=0) fail++;
      data32 = 0x00C00000;
      ret_code = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG, data32 ); //
      if(ret_code!=0) fail++;
      if(isDebug && fail==0) printf("[SIS3316_um]: Done. \n");
      if(isDebug || fail!=0) printf("[SIS3316_um]: Failed %d times! \n",fail);

      // gain/termination 
      if(isDebug) printf("[SIS3316_um]: Setting the gain and termination options... \n");
      fail = 0;
      ret_code = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH1_4_ANALOG_CTRL_REG  ,analog_ctrl_val);
      if(ret_code!=0) fail++;
      ret_code = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH5_8_ANALOG_CTRL_REG  ,analog_ctrl_val);
      if(ret_code!=0) fail++;
      ret_code = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH9_12_ANALOG_CTRL_REG ,analog_ctrl_val);
      if(ret_code!=0) fail++;
      ret_code = CommDriver::vme_write32(vme_handle,SIS3316_ADC_CH13_16_ANALOG_CTRL_REG,analog_ctrl_val);
      if(ret_code!=0) fail++;
      if(isDebug && fail==0) printf("[SIS3316_um]: Done. \n");
      if(isDebug || fail!=0) printf("[SIS3316_um]: Failed %d times! \n",fail);

      if(isDebug) printf("[SIS3316_um]: Intializing the ADC (DAC) offsets... \n");
      u_int32_t adc_dac_offset=0;
      u_int32_t analog_offset_dac_val = 0x8000; // -2.5 < V < 2.5 volts: 32768 (0x8000); 0 < V < 5 volts: 65535; -5 < V < 0 volts: 0 

   }


} // ::SISInterface
