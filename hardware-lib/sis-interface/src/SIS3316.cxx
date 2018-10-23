#include "SIS3316.hh"
//______________________________________________________________________________
SIS3316::SIS3316(sisParameters_t par){
   SetParameters(par);
}
//______________________________________________________________________________
SIS3316::~SIS3316(){
   ClearData(); 
}
//______________________________________________________________________________
int SIS3316::Initialize(){

   // initialization function for the 3316

   // double dt=0;
   // unsigned long *timeStart = (unsigned long *)malloc( sizeof(unsigned long)*6 );
   // unsigned long *timeEnd   = (unsigned long *)malloc( sizeof(unsigned long)*6 );

   // int i=0;
   // for(i=0;i<6;i++) timeStart[i] = 0;
   // for(i=0;i<6;i++) timeEnd[i]   = 0;

   // GetTimeStamp_usec(timeStart);

   // input from user 
   u_int32_t base_addr = fParameters.moduleBaseAddress;
   u_int32_t addr      = 0x0; 
   u_int32_t data32    = 0x0;
   int vme_handle      = fHandle;
   int use_ext_clock   = fParameters.clockType;                  // 0 => false; 1 => true 
   bool isDebug        = fParameters.debug; 

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
   if(use_ext_clock==1)   ClockFreq = (int)fParameters.clockFrequency;

   unsigned int trigger_gate_window_length = (unsigned int)( trigger_gate_window_sec*ClockFreq );   // 

   char msg[512]; 

   if(isDebug){
      std::cout << "[SIS3316::Initialize]: Initializing..." << std::endl;
      sprintf(msg,"[SIS3316::Initialize]: Sampling frequency: %d Hz",ClockFreq);
      std::cout << msg << std::endl;
      sprintf(msg,"[SIS3316::Initialize]: Trigger gate window length: %u (%.4E sec) \n",
            trigger_gate_window_length,trigger_gate_window_sec);
      std::cout << msg << std::endl;
   }

   if(isDebug) std::cout << "[SIS3316::Initialize]: Reading the MOD ID..." << std::endl;
   int rc = GetModuleID();
   if( isDebug && rc==0) std::cout << "[SIS3316::Initialize]: Done."   << std::endl;
   if( isDebug || rc!=0) std::cout << "[SIS3316::Initialize]: Failed!" << std::endl;
   usleep(1);

   if(isDebug) std::cout << "[SIS3316::Initialize]: Issuing key reset..." << std::endl;
   addr = base_addr + SIS3316_KEY_RESET;
   rc = CommDriver::vme_write32(vme_handle,addr, 0x0);
   if(isDebug && rc==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if(isDebug || rc!=0) std::cout << "[SIS3316::Initialize]: Failed! " << std::endl;
   usleep(1);

   if(isDebug) std::cout << "[SIS3316::Initialize]: Issuing key disarm..." << std::endl;
   addr = base_addr + SIS3316_KEY_DISARM; 
   rc = CommDriver::vme_write32(vme_handle,addr, 0x0);
   if(isDebug && rc==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if(isDebug || rc!=0) std::cout << "[SIS3316::Initialize]: Failed! " << std::endl;
   usleep(1);

   if(isDebug) std::cout << "[SIS3316::Initialize]: Configuring the ADC via SPI... " << std::endl;
   rc = adc_spi_setup(vme_handle,adc_125MHz_flag);
   if( isDebug && rc==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if( isDebug || rc!=0) std::cout << "[SIS3316::Initialize]: Failed! " << std::endl;
   usleep(1);

   if(isDebug) std::cout << "[SIS3316::Initialize]: Configuring the clock..." << std::endl;
   rc = configure_clock(use_ext_clock,adc_125MHz_flag);
   if(isDebug && rc==0) std::cout << "[SIS3316::Initialize]: Done." << std::endl;
   if(isDebug || rc!=0) std::cout << "[SIS3316::Initialize]: Failed! " << std::endl;

   if(isDebug) std::cout << "[SIS3316::Initialize]: Turning on the ADC chip outputs... " << std::endl;
   u_int32_t an_offset=0;
   int fail=0;
   for(int i=0;i<4;i++){
      an_offset = i*SIS3316_FPGA_ADC_REG_OFFSET;
      addr = base_addr + an_offset + SIS3316_ADC_CH1_4_SPI_CTRL_REG; 
      rc   = CommDriver::vme_write32(vme_handle,addr, 0x01000000 ); // enable ADC chip outputs
      if(rc!=0) fail++;
   }
   if(isDebug && fail==0) std::cout << "[SIS3316::Initialize]: Done." << std::endl;
   if(isDebug || fail!=0) std::cout << "[SIS3316::Initialize]: Failed " << fail << " times!" << std::endl;
   usleep(1);

   if(isDebug) std::cout << "[SIS3316::Initialize]: Setting the LEMO output 'CO'..." << std::endl;
   data32 = 0x1 ; // Select Sample Clock
   addr = base_addr + SIS3316_LEMO_OUT_CO_SELECT_REG;
   rc = CommDriver::vme_write32(vme_handle,addr, data32 ); //
   if(isDebug && rc==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if(isDebug || rc!=0) std::cout << "[SIS3316::Initialize]: Failed! " << std::endl;

   if(isDebug) std::cout << "[SIS3316::Initialize]: Enabling the LEMO output 'TO'..." << std::endl;
   data32 = 0xffff ; // Select all triggers
   addr = base_addr + SIS3316_LEMO_OUT_TO_SELECT_REG;
   rc = CommDriver::vme_write32(vme_handle,addr, data32 ); //
   if(isDebug && rc==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if(isDebug || rc!=0) std::cout << "[SIS3316::Initialize]: Failed! " << std::endl;

   // header writes 
   if(isDebug) std::cout << "[SIS3316::Initialize]: Setting up the headers..." << std::endl;
   fail = 0;
   data32 = 0x0;
   addr = base_addr + SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG;
   rc = CommDriver::vme_write32(vme_handle,addr, data32 ); //
   if(rc!=0) fail++;
   data32 = 0x00400000;
   addr = base_addr + SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG;
   rc = CommDriver::vme_write32(vme_handle,addr, data32 ); //
   if(rc!=0) fail++;
   data32 = 0x00800000 ;
   addr = base_addr + SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG;
   rc = CommDriver::vme_write32(vme_handle,addr, data32 ); //
   if(rc!=0) fail++;
   data32 = 0x00C00000;
   addr = base_addr + SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG;
   rc = CommDriver::vme_write32(vme_handle,addr, data32 ); //
   if(rc!=0) fail++;
   if(isDebug && fail==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if(isDebug || fail!=0) std::cout << "[SIS3316::Initialize]: Failed " << fail << " times!" << std::endl;

   // gain/termination 
   if(isDebug) std::cout << "[SIS3316::Initialize]: Setting the gain and termination options..." << std::endl;
   fail = 0;
   addr = base_addr + SIS3316_ADC_CH1_4_ANALOG_CTRL_REG;
   rc = CommDriver::vme_write32(vme_handle,addr,analog_ctrl_val);
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH5_8_ANALOG_CTRL_REG;
   rc = CommDriver::vme_write32(vme_handle,addr,analog_ctrl_val);
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH9_12_ANALOG_CTRL_REG;
   rc = CommDriver::vme_write32(vme_handle,addr,analog_ctrl_val);
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH13_16_ANALOG_CTRL_REG;
   rc = CommDriver::vme_write32(vme_handle,addr,analog_ctrl_val);
   if(rc!=0) fail++;
   if(isDebug && fail==0) std::cout << "[SIS3316::Initialize]: Done." << std::endl;
   if(isDebug || fail!=0) std::cout << "[SIS3316::Initialize]: Failed " << fail << " times! " << std::endl;

   if(isDebug) std::cout << "[SIS3316::Initialize]: Intializing the ADC (DAC) offsets... " << std::endl;
   u_int32_t adc_dac_offset=0;
   u_int32_t analog_offset_dac_val = 0x8000; // -2.5 < V < 2.5 volts: 32768 (0x8000); 0 < V < 5 volts: 65535; -5 < V < 0 volts: 0

   //  set ADC offsets (DAC)
   // some details: below in the loop, there are some numbers.  They translate to: 
   // 0x80000000 // DAC CTRL Mode: Write Command
   // 0x2000000  // DAC Command Mode: write to Input
   // 0xf00000   // DAC Address bits: ALL DACs

   if(isDebug) std::cout << "[SIS3316::Initialize]: Enabling the internal reference..." << std::endl;
   fail = 0;
   addr = base_addr + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG;
   rc = CommDriver::vme_write32(vme_handle,addr,0x88f00001);
   if(rc!=0) fail++;
   usleep(50);
   addr = base_addr + SIS3316_ADC_CH5_8_DAC_OFFSET_CTRL_REG;
   rc = CommDriver::vme_write32(vme_handle,addr,0x88f00001);
   if(rc!=0) fail++;
   usleep(50);
   addr = base_addr + SIS3316_ADC_CH9_12_DAC_OFFSET_CTRL_REG;
   rc = CommDriver::vme_write32(vme_handle,addr,0x88f00001);
   if(rc!=0) fail++;
   usleep(50);
   addr = base_addr + SIS3316_ADC_CH13_16_DAC_OFFSET_CTRL_REG;
   rc = CommDriver::vme_write32(vme_handle,addr,0x88f00001);
   if(rc!=0) fail++;
   usleep(50);
   if(isDebug && fail==0) std::cout << "[SIS3316::Initialize]: Done." << std::endl;
   if(isDebug || fail!=0) std::cout << "[SIS3316::Initialize]: Failed " << fail << " times! " << std::endl;;

   if(isDebug) std::cout << "[SIS3316::Initialize]: Now implementing configuration... " << std::endl;
   fail = 0;
   for(int i=0;i<4;i++){ // over all 4 ADC-FPGAs
      adc_dac_offset = i*SIS3316_FPGA_ADC_REG_OFFSET;    //  write cmd?? all DACs??
      addr = base_addr + adc_dac_offset + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG;
      rc = CommDriver::vme_write32(vme_handle,addr,0x80000000 + 0x8000000 +  0xf00000 + 0x1); // set internal reference 
      if(rc!=0) fail++;
      usleep(50); //unsigned int uint_usec   
      // write cmd  write to input  all DACs           offset setting 
      // addr = base_addr + adc_dac_offset + SIS3316_CH1_4_DAC_OFFSET_CTRL_REG; 
      rc = CommDriver::vme_write32(vme_handle,addr,0x80000000 + 0x2000000 +  0xf00000 + ((analog_offset_dac_val & 0xffff) << 4) );  //
      if(rc!=0) fail++;
      usleep(50); //unsigned int uint_usec??
      rc = CommDriver::vme_write32(vme_handle,addr,0xC0000000 );  //
      if(rc!=0) fail++;
      usleep(50); //unsigned int uint_usec
   }
   if(isDebug && fail==0) std::cout << "[SIS3316::Initialize]: Done. "  << std::endl;
   if(isDebug || fail!=0) std::cout << "[SIS3316::Initialize]: Failed " << fail << " times! " << std::endl; 

   if(isDebug) std::cout << "[SIS3316::Initialize]: Setting the trigger gate window length register..." << std::endl;
   fail = 0;
   data32 = (trigger_gate_window_length - 2) & 0xffff;
   addr = base_addr + SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH5_8_TRIGGER_GATE_WINDOW_LENGTH_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH9_12_TRIGGER_GATE_WINDOW_LENGTH_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH13_16_TRIGGER_GATE_WINDOW_LENGTH_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(rc!=0) fail++;
   if(isDebug && fail==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if(isDebug || fail!=0) std::cout << "[SIS3316::Initialize]: Failed " << fail << " times! " << std::endl;

   if(isDebug) std::cout << "[SIS3316::Initialize]: Setting the pre-trigger delay value... " << std::endl;
   fail = 0;
   data32 = 0x0; // 2042;
   addr = base_addr + SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG;  
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG;  
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG;  
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG;  
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(rc!=0) fail++;
   if(isDebug && fail==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if(isDebug || fail!=0) std::cout << "[SIS3316::Initialize]: Failed " << fail << " times!" << std::endl;

   // Disable/Enable LEMO Input "TI" as External Trigger
   if(isDebug) std::cout << "[SIS3316::Initialize]: Setting the trigger type... " << std::endl;
   if (auto_trigger_enable==1) {
      data32 = 0x0;  // Disable NIM Input "TI"
   }else{
      data32 = 0x10; // Enable NIM Input "TI"
   }

   addr = base_addr + SIS3316_NIM_INPUT_CONTROL_REG;
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(isDebug && rc==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if(isDebug || rc!=0) std::cout << "[SIS3316::Initialize]: Failed! " << std::endl;

   if(isDebug) std::cout << "[SIS3316::Initialize]: Enabling external triggers... " << std::endl;
   // data32 = 0x100; // external trigger function as trigger enable   
   // data32 = 0x400; // external timestamp clear enabled  
   data32 = 0x500; // external trigger function as trigger enable + external timestamp clear enabled  
   //data32 = 0x0;
   addr = base_addr + SIS3316_ACQUISITION_CONTROL_STATUS;
   rc = CommDriver::vme_write32(vme_handle,addr,data32);
   if(isDebug && fail==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if(isDebug || fail!=0) std::cout << "[SIS3316::Initialize]: Failed " << fail << " times! " << std::endl;
   usleep(1);

   if(isDebug) std::cout << "[SIS3316::Initialize]: Clearing the timestamp... " << std::endl;
   addr = base_addr + SIS3316_KEY_TIMESTAMP_CLEAR;
   rc = CommDriver::vme_write32(vme_handle,addr,0x0);
   if(isDebug && fail==0) std::cout << "[SIS3316::Initialize]: Done. " << std::endl;
   if(isDebug || fail!=0) std::cout << "[SIS3316::Initialize]: Failed " << fail << " times! " << std::endl;
   usleep(5);        // it's probably best to wait a bit before starting... 

   if(rc!=0) std::cout << "[SIS3316::Initialize]: Initialization failed! " << std::endl;

   return rc;
}
//______________________________________________________________________________
int SIS3316::ReInitialize(){
   // re-initialize the digitizer in the case that the event length has changed

   // input from user 
   int vme_handle                   = fHandle;
   u_int32_t base_addr              = fParameters.moduleBaseAddress;  
   u_int32_t input_nof_samples      = (u_int32_t)fParameters.numberOfSamples; // number of samples  
   u_int32_t NEvents                = (u_int32_t)fParameters.numberOfEvents;  // number of events 
   u_int32_t event_length           = input_nof_samples;                      // number of samples per event
   unsigned long int NEventsOnADC   = 1;                                      // print 1 event to file; make the addr thresh 1 event.
   bool isDebug                     = fParameters.debug; 

   u_int32_t addr=0x0; 

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

   // bookkeeping
   double input_nof_samples_mb      = ( (double)input_nof_samples )*sample_size_bytes/1E+6;

   char msg[512]; 

   if(input_nof_samples>raw_buf_max){
      use_ext_raw_buf      = 1;
      ext_raw_data_buf_reg = event_length;
      sprintf(msg,"[SIS3316::ReInitialize]: Using EXTENDED raw data buffer (number of samples = %lu)",
            (unsigned long)input_nof_samples);
      std::cout << msg << std::endl;
   }else{
      // ANDed with 1s to make sure it's 16 bits wide; 
      // bit-shifted to the left by 16 to meet register requirements 
      raw_data_buf_reg     = (input_nof_samples & 0xffff) << 16;
   }

   if(input_nof_samples>tot_buf_max){
      if(isDebug) std::cout << "[SIS3316::ReInitialize]: Number of samples too big!  Setting to maximum... " << std::endl;
      use_ext_raw_buf      = 1;
      ext_raw_data_buf_reg = tot_buf_max - 1;
   }

   // FIXME: Should be in number of 32-bit words!
   unsigned long int addr_thresh        = (unsigned long int)( NEventsOnADC*event_length/2 );   
   unsigned long int max_read_nof_words = NEventsOnADC*event_length;

   std::cout << "[SIS3316::ReInitialize]: Initializing..." << std::endl;

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

   if(isDebug) std::cout << "[SIS3316::ReInitialize]: Issuing key disarm..." << std::endl;
   addr = base_addr + SIS3316_KEY_DISARM;
   rc = CommDriver::vme_write32(vme_handle,addr, 0x0);
   if(isDebug && rc==0) std::cout << "[SIS3316::ReInitialize]: Done. " << std::endl;
   if(isDebug || rc!=0) std::cout << "[SIS3316::ReInitialize]: Failed!" << std::endl;
   usleep(1);

   if(use_ext_raw_buf==0){
      if(isDebug) std::cout << "[SIS3316::ReInitialize]: Writing data to raw data buffer config register... " << std::endl;
      fail = 0;
      addr = base_addr + SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG;
      rc = CommDriver::vme_write32(vme_handle,addr,raw_data_buf_reg);
      if(rc!=0) fail++;
      addr = base_addr + SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG;
      rc = CommDriver::vme_write32(vme_handle,addr,raw_data_buf_reg);
      if(rc!=0) fail++;
      addr = base_addr + SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG;
      rc = CommDriver::vme_write32(vme_handle,addr,raw_data_buf_reg);
      if(rc!=0) fail++;
      addr = base_addr + SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG;
      rc = CommDriver::vme_write32(vme_handle,addr,raw_data_buf_reg);
      if(rc!=0) fail++;
      if(isDebug && fail==0) std::cout << "[SIS3316::ReInitialize]: Done. " << std::endl;
      if(isDebug || fail!=0) std::cout << "[SIS3316::ReInitialize]: Failed " << fail << " times!" << std::endl;
      std::cout << "[SIS3316::ReInitialize]: Reading data from raw data buffer config register... " << std::endl;
      fail = 0;
      addr = base_addr + SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG; 
      rc  = CommDriver::vme_read32(vme_handle,addr,&read_data);
      if(rc!=0) fail++;
      data_low  =  read_data & 0x0000ffff;                 // low bytes 
      data_high = (read_data & 0xffff0000)/pow(2,16);      // high bytes 
      sum       =  data_low + data_high;
      if(isDebug){
         sprintf(msg,"[SIS3316::ReInitialize]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
         std::cout << msg << std::endl;
      }
      // std::cout << "low bytes:  %lu \n",data_low);  
      // std::cout << "high bytes: %lu \n",data_high);  
      // std::cout << "sum:        %lu \n",sum);  
      addr = base_addr + SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG; 
      rc  = CommDriver::vme_read32(vme_handle,addr,&read_data);
      if(rc!=0) fail++;
      data_low  =  read_data & 0x0000ffff;                 // low bytes 
      data_high = (read_data & 0xffff0000)/pow(2,16);      // high bytes 
      sum       =  data_low + data_high;
      if(isDebug){
         sprintf(msg,"[SIS3316::ReInitialize]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
         std::cout << msg << std::endl;
      }
      // std::cout << "low bytes:  %lu \n",data_low);  
      // std::cout << "high bytes: %lu \n",data_high);  
      // std::cout << "sum:        %lu \n",sum);  
      addr = base_addr + SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG; 
      rc  = CommDriver::vme_read32(vme_handle,addr,&read_data);
      if(rc!=0) fail++;
      data_low  =  read_data & 0x0000ffff;                 // low bytes 
      data_high = (read_data & 0xffff0000)/pow(2,16);      // high bytes 
      sum       =  data_low + data_high;
      if(isDebug){
         sprintf(msg,"[SIS3316::ReInitialize]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
         std::cout << msg << std::endl;
      }
      // std::cout << "low bytes:  %lu \n",data_low);  
      // std::cout << "high bytes: %lu \n",data_high);  
      // std::cout << "sum:        %lu \n",sum); 
      addr = base_addr + SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG; 
      rc  = CommDriver::vme_read32(vme_handle,addr,&read_data);
      if(rc!=0) fail++;
      data_low  =  read_data & 0x0000ffff;                 // low bytes 
      data_high = (read_data & 0xffff0000)/pow(2,16);      // high bytes 
      sum       =  data_low + data_high;
      if(isDebug){
         sprintf(msg,"[SIS3316::ReInitialize]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
      }
      // std::cout << "low bytes:  %lu \n",data_low);  
      // std::cout << "high bytes: %lu \n",data_high);  
      // std::cout << "sum:        %lu \n",sum);  
      if(isDebug && fail==0) std::cout << "[SIS3316::ReInitialize]: Done. " << std::endl;
      if(isDebug || fail!=0) std::cout << "[SIS3316::ReInitialize]: Failed " << fail << " times! " << std::endl;
   }else if(use_ext_raw_buf==1){
      if(isDebug) std::cout << "[SIS3316::ReInitialize]: Writing data to EXTENDED raw data buffer config register..." << std::endl; 
      fail = 0;
      addr = base_addr + SIS3316_ADC_CH1_4_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG;
      rc = CommDriver::vme_write32(vme_handle,addr,ext_raw_data_buf_reg);
      if(rc!=0) fail++;
      addr = base_addr + SIS3316_ADC_CH5_8_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG;
      rc = CommDriver::vme_write32(vme_handle,addr,ext_raw_data_buf_reg);
      if(rc!=0) fail++;
      addr = base_addr + SIS3316_ADC_CH9_12_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG;
      rc = CommDriver::vme_write32(vme_handle,addr,ext_raw_data_buf_reg);
      if(rc!=0) fail++;
      addr = base_addr + SIS3316_ADC_CH13_16_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG;
      rc = CommDriver::vme_write32(vme_handle,addr,ext_raw_data_buf_reg);
      if(rc!=0) fail++;
      if(isDebug && fail==0) std::cout << "[SIS3316::ReInitialize]: Done. " << std::endl;
      if(isDebug || fail!=0) std::cout << "[SIS3316::ReInitialize]: Failed " << fail << " times! " << std::endl;
      if(isDebug) std::cout << "[SIS3316::ReInitialize]: Reading data from EXTENDED raw data buffer config register... " << std::endl;
      fail = 0;
      addr = base_addr + SIS3316_ADC_CH1_4_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG;
      rc  = CommDriver::vme_read32(vme_handle,addr,&read_data);
      if(rc!=0) fail++;
      data_low  =  read_data & 0x00000fff;                 // low bytes 
      data_high = (read_data & 0x00fff000)/pow(2,12);      // high bytes 
      sum       = data_low + data_high;
      if(isDebug){
         sprintf(msg,"[SIS3316::ReInitialize]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
         std::cout << msg << std::endl;
      }
      // std::cout << "low bytes:  %lu \n",data_low);  
      // std::cout << "high bytes: %lu \n",data_high);  
      // std::cout << "sum:        %lu \n",sum);  
      addr = base_addr + SIS3316_ADC_CH5_8_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG;
      rc  = CommDriver::vme_read32(vme_handle,addr,&read_data);
      if(rc!=0) fail++;
      data_low  =  read_data & 0x00000fff;                 // low bytes 
      data_high = (read_data & 0x00fff000)/pow(2,12);      // high bytes 
      sum       = data_low + data_high;
      if(isDebug){
         sprintf(msg,"[SIS3316::ReInitialize]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
      }
      // std::cout << "low bytes:  %lu \n",data_low);  
      // std::cout << "high bytes: %lu \n",data_high);  
      // std::cout << "sum:        %lu \n",sum); 
      addr = base_addr + SIS3316_ADC_CH9_12_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG; 
      rc  = CommDriver::vme_read32(vme_handle,addr,&read_data);
      if(rc!=0) fail++;
      data_low  =  read_data & 0x00000fff;                 // low bytes 
      data_high = (read_data & 0x00fff000)/pow(2,12);      // high bytes 
      sum       = data_low + data_high;
      if(isDebug){
         sprintf(msg,"[SIS3316::ReInitialize]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
         std::cout << msg << std::endl;
      }
      // std::cout << "low bytes:  %lu \n",data_low);  
      // std::cout << "high bytes: %lu \n",data_high);  
      // std::cout << "sum:        %lu \n",sum);  
      addr = base_addr + SIS3316_ADC_CH13_16_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG; 
      rc  = CommDriver::vme_read32(vme_handle,addr,&read_data);
      if(rc!=0) fail++;
      data_low  =  read_data & 0x00000fff;                 // low bytes 
      data_high = (read_data & 0x00fff000)/pow(2,12);      // high bytes 
      sum       = data_low + data_high;
      if(isDebug){
         sprintf(msg,"[SIS3316::ReInitialize]: read data:  %lu (hex: 0x%08x) \n",(unsigned long)read_data,read_data);
         std::cout << msg << std::endl;
      }
      // std::cout << "low bytes:  %lu \n",data_low);  
      // std::cout << "high bytes: %lu \n",data_high);  
      // std::cout << "sum:        %lu \n",sum);  
      if(isDebug && fail==0) std::cout << "[SIS3316::ReInitialize]: Done. " << std::endl;
      if(isDebug || fail!=0) std::cout << "[SIS3316::ReInitialize]: Failed " << fail << " times! " << std::endl;

      if(rc!=0) std::cout << "[SIS3316::ReInitialize]: Failed! " << std::endl;
   }
   return rc;
}
//______________________________________________________________________________
int SIS3316::configure_clock(int use_ext_clock,int adc_125MHz_flag){
   int rc = 0;
   int fail = 0;

   char msg[512]; 

   int vme_handle      = fHandle;
   u_int32_t base_addr = fParameters.moduleBaseAddress; 
   u_int32_t addr      = 0x0; 

   bool isDebug        = fParameters.debug; 

   unsigned int clock_N1div = 0;
   unsigned int clock_HSdiv = 0;
   unsigned int iob_delay_value = 0;

   u_int32_t read_data = 0;

   int ClockFreq=0;
   int ClockFreq_in_units=0;

   std::string units = fParameters.clockFreqUnits; 

   if(use_ext_clock==1){
      ClockFreq          = (int)fParameters.clockFrequency;
      units              = fParameters.clockFreqUnits;
      ClockFreq_in_units = 0;
      if( units.compare("kHz")==0 ) ClockFreq_in_units = ClockFreq/1E+3;
      if( units.compare("MHz")==0 ) ClockFreq_in_units = ClockFreq/1E+6;
      if( units.compare("GHz")==0 ) ClockFreq_in_units = ClockFreq/1E+9;
   }

   if(use_ext_clock==0){
      if(adc_125MHz_flag==0){
         // 250  MHz
         if(isDebug) std::cout << "[SIS3316::configure_clock]: Using internal clock: 250 MHz..." << std::endl;
         clock_N1div      =  4  ;
         clock_HSdiv      =  5  ;
         // iob_delay_value  =  0x48   ; // ADC FPGA version A_0250_0003
         iob_delay_value  =  0x1008 ; // ADC FPGA version A_0250_0004 and higher
      }else{
         if(isDebug) std::cout << "[SIS3316::configure_clock]: Using internal clock: 125 MHz..." << std::endl;
         clock_N1div      =  8  ;
         clock_HSdiv      =  5  ;
         // iob_delay_value  =  0x7F   ; // ADC FPGA version A_0250_0003
         iob_delay_value  =  0x1020 ; // ADC FPGA version A_0250_0004 and higher
      }
      rc = change_frequency_HSdiv_N1div(vme_handle,0,clock_HSdiv,clock_N1div);
      if(isDebug && rc==0) std::cout << "[SIS3316::configure_clock]: Done." << std::endl;
      if(isDebug || rc!=0) std::cout << "[SIS3316::configure_clock]: Failed!" << std::endl;
   }else if(use_ext_clock==1){
      iob_delay_value = 0x0;
      if(isDebug){
         sprintf(msg,"[SIS3316::configure_clock]: Using external clock: %d %s",ClockFreq_in_units,units.c_str());
         std::cout << msg << std::endl;
         sprintf(msg,"[SIS3316::configure_clock]: Setting sample clock distribution control to 3 (external lemo)...\n");
         std::cout << msg << std::endl;
      }
      addr = base_addr + SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL; 
      rc = CommDriver::vme_write32(vme_handle,addr, 0x3);
      if(isDebug && rc==0) std::cout << "[SIS3316::configure_clock]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SIS3316::configure_clock]: Failed!" << std::endl;
      rc = CommDriver::vme_read32(vme_handle,addr,&read_data);
      if(isDebug){
         sprintf(msg,"[SIS3316::configure_clock]: Read sample clock distribution: 0x%08x\n", read_data);
         std::cout << msg << std::endl;
      }
      if(isDebug && rc==0) std::cout << "[SIS3316::configure_clock]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SIS3316::configure_clock]: Failed!" << std::endl;

      if(isDebug) std::cout << "[SIS3316::configure_clock]: Bypassing precision clock multiplier..." << std::endl;
      rc = vme_write_si5325(vme_handle, 0, 0x2);
      if(isDebug && rc==0) std::cout << "[SIS3316::configure_clock]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SIS3316::configure_clock]: Failed! " << std::endl;

      if(isDebug) std::cout << "[SIS3316::configure_clock]: Powering down the old clock..." << std::endl;
      rc = vme_write_si5325(vme_handle, 11, 0x02);
      if(isDebug && rc==0) std::cout << "[SIS3316::configure_clock]: Done. " << std::endl;
      if(isDebug || rc!=0) std::cout << "[SIS3316::configure_clock]: Failed! " << std::endl;
   }
   usleep(1000);

   if(isDebug) std::cout << "[SIS3316::configure_clock]: Resetting the DCM/PLL of all FPGAs... " << std::endl;
   addr = base_addr + SIS3316_KEY_ADC_CLOCK_DCM_RESET; 
   rc = CommDriver::vme_write32(vme_handle,addr,0x0);
   if(isDebug && rc==0) std::cout << "[SIS3316::configure_clock]: Done. " << std::endl;
   if(isDebug || rc!=0) std::cout << "[SIS3316::configure_clock]: Failed! " << std::endl;
   usleep(10000);   // wait 10 ms for clock to be stable; should be fine after 5 ms, but we wait longer  

   fail = 0;
   if(isDebug) std::cout << "[SIS3316::configure_clock]: Setting the input tap delays... " << std::endl;
   addr = base_addr + SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr, 0xf00 ); // Calibrate IOB _delay Logic
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr, 0xf00 ); // Calibrate IOB _delay Logic
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr, 0xf00 ); // Calibrate IOB _delay Logic
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr, 0xf00 ); // Calibrate IOB _delay Logic
   if(rc!=0) fail++;
   usleep(1000) ;
   addr = base_addr + SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr, 0x300 + iob_delay_value ); // set IOB _delay Logic
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr, 0x300 + iob_delay_value ); // set IOB _delay Logic
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr , 0x300 + iob_delay_value ); // set IOB _delay Logic
   if(rc!=0) fail++;
   addr = base_addr + SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr, 0x300 + iob_delay_value ); // set IOB _delay Logic
   if(rc!=0) fail++;
   usleep(1000) ;

   if(isDebug && fail==0) std::cout << "[SIS3316::configure_clock]: Done. " << std::endl;
   if(isDebug || fail!=0) std::cout << "[SIS3316::configure_clock]: Failed " << fail << " times! " << std::endl;

   return rc;
}
//______________________________________________________________________________
int SIS3316::adc_spi_setup(int vme_handle,int adc_125MHz_flag){
   // this needs to be done to turn on all the ADC outputs properly.
   // pulled from the sis3316_class.cpp file.  

   // adc_fpga_group: 0,1,2,3
   // adc_chip: 0 or 1
   //  -1 : not all adc chips have the same chip ID
   //  >0 : VME Error Code

   int rc;
   unsigned int adc_chip_id;
   unsigned int addr, data;

   u_int32_t base_addr = fParameters.moduleBaseAddress; 

   char msg[512]; 

   // disable ADC output
   for(unsigned i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++) {
      addr = base_addr + SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((i_adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
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
         // sprintf(msg,"i_adc_fpga_group = %d i_adc_chip = %d data = 0x%08x adc_chip_id = 0x%08x", 
         //           i_adc_fpga_group, i_adc_chip, data, adc_chip_id);
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
      addr = base_addr + SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((i_adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
      rc = CommDriver::vme_write32(vme_handle,addr,0x1000000); //  set bit 24
      if (rc != 0) {return rc ; }
   }

   return 0; 
}
//______________________________________________________________________________
int SIS3316::adc_spi_write(int vme_handle,unsigned int adc_fpga_group,unsigned int adc_chip,unsigned int spi_addr,unsigned int spi_data){
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

   u_int32_t base_addr = fParameters.moduleBaseAddress; 

   // read register to get the information of bit 24 (adc output enabled)
   addr = base_addr + SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
   rc = CommDriver::vme_read32(vme_handle,addr,&data); //  

   if(rc!=0){
      return rc; 
   }

   data = data & 0x01000000 ; // save bit 24
   data = data + 0x80000000 + uint_adc_mux_select + ((spi_addr & 0xffff) << 8) + (spi_data & 0xff) ;
   addr = base_addr + SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
   rc   = CommDriver::vme_write32(vme_handle,addr,data);
   //usleep(1000); 

   addr = base_addr + SIS3316_ADC_FPGA_SPI_BUSY_STATUS_REG;

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
int SIS3316::adc_spi_read(int vme_handle,unsigned int adc_fpga_group,unsigned int adc_chip,unsigned int spi_addr,unsigned int *spi_data){
   // this needs to be done to turn on all the ADC outputs properly.
   // pulled from the sis3316_class.cpp file. 

   char msg[512]; 
   int rc;
   unsigned int data ;
   unsigned int addr ;
   unsigned int uint_adc_mux_select;
   unsigned int pollcounter;

   u_int32_t base_addr = fParameters.moduleBaseAddress; 

   bool isDebug = fParameters.debug; 

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

   addr = base_addr + SIS3316_ADC_CH1_4_SPI_CTRL_REG +  ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ;
   rc = CommDriver::vme_write32(vme_handle,addr,data); //

   if(isDebug){
      if (rc!=0) {
         sprintf(msg,"adc_spi_read vme_A32D32_write 1: rc = 0x%08x", rc);
         std::cout << msg << std::endl;
      }
   }

   addr = base_addr + SIS3316_ADC_FPGA_SPI_BUSY_STATUS_REG;

   do{
      rc = CommDriver::vme_read32(vme_handle,addr,&data); 
      if(isDebug){
         if(rc!=0){
            sprintf(msg,"[SIS3316::adc_spi_read]: vme_A32D32_read 2: rc = 0x%08x",rc);
            std::cout << msg << std::endl;
         }
      }
      pollcounter--;
      //} while (((data & 0x80000000) == 0x80000000) && (pollcounter > 0) && (rc == 0)); // VME FPGA Version 0x0006 and higher
   }while( ((data&0x0000000f)!=0x00000000)&&(pollcounter>0)&&(rc==0) ); // changed 2.12.2014,  VME FPGA Version 0x0005 and lower

   if(isDebug){
      sprintf(msg,"adc_spi_read pollcounter: pollcounter = 0x%08x", pollcounter);
      std::cout << msg << std::endl;
   }

   if(rc!=0){
      return rc;
   }
   
   if(pollcounter==0){
      return -2;
   }

   usleep(20); //

   //addr = SIS3316_ADC_CH1_4_SPI_READBACK_REG  ; // removed 21.01.2015
   addr = base_addr + SIS3316_ADC_CH1_4_SPI_READBACK_REG + ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) ; // changed 21.01.2015
   rc = CommDriver::vme_read32(vme_handle,addr,&data);
   
   if(isDebug){
      if(rc != 0){
         sprintf(msg,"adc_spi_read vme_A32D32_read 3: rc = 0x%08x", rc);
         std::cout << msg << std::endl;
      }
   }
   
   if (rc != 0) {return rc ; }
   
   *spi_data = data & 0xff ;
   
   return 0;
}
//_____________________________________________________________________________
int SIS3316::vme_write_si5325(int vme_handle, u_int32_t si5325_addr, u_int32_t data32){

   int rc, poll_counter;
   u_int32_t write_data, read_data,addr;
   int POLL_COUNTER_MAX = 100;

   u_int32_t base_addr = fParameters.moduleBaseAddress;
   // bool isDebug        = fParameters.debug;   

   // tell SI5325 what address we want to write to
   write_data = 0x0000 + (si5325_addr & 0xff);
   addr = base_addr + SIS3316_NIM_CLK_MULTIPLIER_SPI_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr,write_data);
   if (rc != 0) { return -1; }
   usleep(10000);

   // confirm that the SI5325 understood/processed our request to set the write address
   poll_counter = 0;
   do{
      poll_counter++; 
      addr = base_addr + SIS3316_NIM_CLK_MULTIPLIER_SPI_REG; 
      CommDriver::vme_read32(vme_handle,addr, &read_data);
   }while ( ( (read_data & 0x80000000) == 0x80000000) && (poll_counter < POLL_COUNTER_MAX) );
   if (poll_counter == POLL_COUNTER_MAX) {      return -2 ;     }

   // actually write data to the specificed SI5325 address
   write_data = 0x4000 + (data32 & 0xff);
   addr = base_addr + SIS3316_NIM_CLK_MULTIPLIER_SPI_REG; 
   rc = CommDriver::vme_write32(vme_handle,addr, write_data);
   if (rc != 0) { return -1; }
   usleep(10000);

   // again confirm that the SI5325 understood/processed our requested write
   poll_counter = 0;
   do{
      poll_counter++;
      addr = base_addr + SIS3316_NIM_CLK_MULTIPLIER_SPI_REG; 
      CommDriver::vme_read32(vme_handle,addr, &read_data);
   }while ( ( (read_data & 0x80000000) == 0x80000000) && (poll_counter < POLL_COUNTER_MAX) );

   if(poll_counter == POLL_COUNTER_MAX){
      return -2;
   }

   return 0;
}
//_____________________________________________________________________________
int SIS3316::change_frequency_HSdiv_N1div(int vme_handle,int osc, unsigned hs_div_val, unsigned n1_div_val){

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
int SIS3316::set_frequency(int vme_handle,int osc, unsigned char *values){

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
   u_int32_t base_addr = fParameters.moduleBaseAddress;
   u_int32_t addr      = base_addr + 0x438;  
   rc = CommDriver::vme_write32(vme_handle,addr,0);

   if(rc){
      return rc;
   }

   // DCM Reset -> the DCM/PLL of the ADC-FPGAs will be stable after max. 5ms
   //              or check the DCM OK bits (ADC FPGA Status registers bit 20)
   usleep(5000); // 5 ms

   return 0;
}
//______________________________________________________________________________
int SIS3316::Si570ReadDivider(int vme_handle,int osc, unsigned char *data){

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
int SIS3316::Si570Divider(int vme_handle,int osc, unsigned char *data){

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
int SIS3316::Si570FreezeDCO(int vme_handle,int osc){

   int rc;
   char ack;

   // start
   rc = I2cStart(vme_handle,osc);

   if(rc){
      I2cStop(vme_handle,osc);
      return rc;
   }

   // address
   rc = I2cWriteByte(vme_handle,osc,OSC_ADR<<1,&ack);
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
   rc = I2cWriteByte(vme_handle,osc, 0x10, &ack);
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
int SIS3316::Si570UnfreezeDCO(int vme_handle,int osc){

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
int SIS3316::Si570NewFreq(int vme_handle,int osc){

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
int SIS3316::I2cStart(int vme_handle,int osc){

   int rc;
   int i;
   unsigned int tmp;

   if(osc > 3){
      return -101;
   }

   u_int32_t base_addr = fParameters.moduleBaseAddress;
   u_int32_t addr      = 0x0; 

   // start
   addr = base_addr + SIS3316_ADC_CLK_OSC_I2C_REG + (4*osc); 
   rc = CommDriver::vme_write32(vme_handle,addr,1<<I2C_START);

   if(rc){
      return rc;
   }

   i = 0;

   do{
      // poll i2c fsm busy
      addr = base_addr + SIS3316_ADC_CLK_OSC_I2C_REG + (4*osc); 
      rc = CommDriver::vme_read32(vme_handle,addr, &tmp);
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
int SIS3316::I2cStop(int vme_handle,int osc){

   int rc;
   int i;
   unsigned int tmp;

   if(osc > 3){
      return -101;
   }

   u_int32_t base_addr = fParameters.moduleBaseAddress;
   u_int32_t addr      = 0x0; 

   // stop
   addr = base_addr + SIS3316_ADC_CLK_OSC_I2C_REG + (4*osc); 
   rc = CommDriver::vme_write32(vme_handle,addr,1<<I2C_STOP);
   if(rc){
      return rc;
   }

   i = 0;
   do{
      // poll i2c fsm busy
      addr = base_addr + SIS3316_ADC_CLK_OSC_I2C_REG + (4*osc); 
      rc = CommDriver::vme_read32(vme_handle,addr, &tmp);
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
int SIS3316::I2cWriteByte(int vme_handle,int osc, unsigned char data, char *ack){

   int rc;
   int i;
   unsigned int tmp;

   if(osc > 3){
      return -101;
   }

   u_int32_t base_addr = fParameters.moduleBaseAddress;
   u_int32_t addr      = 0x0; 

   // write byte, receive ack
   addr = base_addr + SIS3316_ADC_CLK_OSC_I2C_REG + (4*osc); 
   rc = CommDriver::vme_write32(vme_handle,addr, 1<<I2C_WRITE ^ data);
   if(rc){
      return rc;
   }

   i = 0;
   do{
      // poll i2c fsm busy
      addr = base_addr + SIS3316_ADC_CLK_OSC_I2C_REG + (4*osc); 
      rc = CommDriver::vme_read32(vme_handle,addr, &tmp);
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
int SIS3316::I2cReadByte(int vme_handle,int osc, unsigned char *data, char ack){

   int rc;
   int i;
   unsigned int tmp;
   unsigned char char_tmp;

   if(osc > 3){
      return -101;
   }

   u_int32_t base_addr = fParameters.moduleBaseAddress;
   u_int32_t addr      = 0x0;

   // read byte, put ack
   tmp = 1<<I2C_READ;
   tmp |= ack ? 1<<I2C_ACK : 0;
   addr = base_addr + SIS3316_ADC_CLK_OSC_I2C_REG + (4*osc); 
   rc = CommDriver::vme_write32(vme_handle,addr, tmp);
   if(rc){
      return rc;
   }

   i = 0;
   do{
      // poll i2c fsm busy
      addr = base_addr + SIS3316_ADC_CLK_OSC_I2C_REG + (4*osc); 
      rc = CommDriver::vme_read32(vme_handle,addr, &tmp);
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
int SIS3316::call_vme_A32MBLT64FIFO_read(int vme_handle, u_int32_t vme_adr, u_int32_t* vme_data,
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
int SIS3316::read_DMA_Channel_PreviousBankDataBuffer(int vme_handle,                    /* VME handle */
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

   u_int32_t base_addr = fParameters.moduleBaseAddress;

   bool isDebug        = fParameters.debug; 

   // read previous Bank sample address
   // poll until it is valid.
   addr = base_addr + SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG 
        + ((channel_no & 0x3) * 4) + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
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
         sprintf(msg,"[SIS3316]: Error: max_poll_counter = 0x%08x ", max_poll_counter);
         std::cout << msg << std::endl;
         sprintf(msg,"[SIS3316]: previous bank address value: 0x%08x",previous_bank_addr_value);
         std::cout << msg << std::endl;
         return 0x900;
      }

   }while( ( (previous_bank_addr_value & 0x1000000) >> 24 )  != bank2_read_flag ); // previous Bank sample address is valid if bit 24 is equal bank2_read_flag 

   if(isDebug) sprintf(msg,"[SIS3316]: previous bank address value: 0x%08x \n",previous_bank_addr_value);
   std::cout << msg << std::endl;
   if(isDebug) sprintf(msg,"[SIS3316]: addr = 0x%08x previous bank address value = 0x%08x \n",SIS3316_MOD_BASE+addr,previous_bank_addr_value);
   std::cout << msg << std::endl;

   // check the obtained previous bank address; return zero if
   // we have nothing. 
   if( (previous_bank_addr_value & 0xffffff)==0 ){ // no data sampled !
      std::cout << "[SIS3316]: No data sampled!" << std::endl;
      sprintf(msg,"[SIS3316]: addr = 0x%08x previous bank address value = 0x%08x \n",SIS3316_MOD_BASE+addr,previous_bank_addr_value);
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
      addr        = base_addr + SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + ( ( (channel_no >> 2) & 0x3) * 4 );
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
         addr                = base_addr + SIS3316_MOD_BASE + SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3 )* SIS3316_FPGA_ADC_MEM_OFFSET);
         req_nof_32bit_words = previous_bank_addr_value & 0xffffff ;  // get the lowest 24 bits 
         // printf("number of 32-bit words: %d (0x%08x) \n",req_nof_32bit_words,req_nof_32bit_words); 

         if(req_nof_32bit_words > max_read_nof_words){
            sprintf(msg,"[SIS3316]: Error: exceeded the allowed number of data words! Setting to maximum = %u.",max_read_nof_words);
            std::cout << msg << std::endl;
            req_nof_32bit_words = max_read_nof_words;
         }
         usleep(5); // wait 5 us before reading out data 
         // FIFORead(vme_handle,addr,NumOfSamples);  
         // FIFO: first in, first out 
         rc = call_vme_A32MBLT64FIFO_read(vme_handle,addr,uint_adc_buffer,((req_nof_32bit_words + 1) & 0xfffffE),&got_nof_32bit_words); // N * 8-byte length  !!! 

         if(rc != 0){
            sprintf(msg,"[SIS3316]: Error: vme_A32MBLT64FIFO_read: rc = 0x%08x   addr = 0x%08x  req_nof_32bit_words = 0x%08x",
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
            sprintf(msg,"[SIS3316]: Info: retry_counter = %d",retry_counter);
         }
      }

      // reset FSM again
      addr = base_addr + SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4) ;
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
      sprintf(msg,"[SIS3316]: Tried too many times.");
      std::cout << msg << std::endl;
      return -1 ;
   }
   return 0 ;
}
//______________________________________________________________________________
int SIS3316::ReadOutData(){

   // Samples an NMR pulse when called.  After the address threshold is reached 
   // for a single event (i.e., pulse), the current memory bank is disarmed and the idle 
   // one is armed.  From here, the data is written to file, where the pulse number (EventNum)
   // serves as the file name, stored in the appropriate output directory (output_dir). 
   // inputs:
   // - vme_handle:      VME handle
   // - myADC:           ADC struct with all ADC settings 
   // - output_dir:      Output directory path 
   // - EventNum:        The current event number 
   // - armed_bank_flag: The currently armed bank (0 = bank2, 1 = bank1) 
   // outputs 
   // - armed_bank_flag: Is updated to reflect currently armed bank
   // - rc: reflects status of the sampling; 0 => success; -97: no data read; -98: addr threshold not reached. 

   int vme_handle      = fHandle; 
   bool isDebug        = fParameters.debug; 
   u_int32_t base_addr = fParameters.moduleBaseAddress;
   u_int32_t addr      = 0x0;  

   // if(isDebug) printf("[SIS3316::ReadOutData]: ARMED BANK FLAG: %d \n",*armed_bank_flag);

   int EventNum             = 1; 
   int rc                   = 0;
   int bank1_armed_flag     = -1;
   int input_nof_samples    = fParameters.numberOfSamples;
   int start_ch             = fParameters.channelNumber - 1;  // ADC channel number; subtract 1 because index runs from 0,..,15. 
   int end_ch               = start_ch + 1;
   // int start_ch             = 0; 
   // int end_ch               = 1; // SIS3316_MAX_CH; 
   // const int SIS3316_MAX_CH = 16; 
   int max_read_nof_words   = input_nof_samples;
   int poll_counter         = 0;
   int poll_counter_max     = 10000;
   const long int SIZE      = max_read_nof_words;

   unsigned int i=0;
   unsigned int got_nof_32bit_words = 0;

   char msg[512]; 

   // apparently this works better for large arrays...
   u_int32_t *adc_buffer    = static_cast<u_int32_t *>( malloc( sizeof(u_int32_t)*SIZE     ) );
   // u_int16_t *adc_buffer_us = static_cast<u_int16_t *>( malloc( sizeof(u_int16_t)*(2*SIZE) ) );

   fData.resize(SIZE);  

   u_int32_t read_data=0,read_data_2=0,addr_thresh=0;
   u_int32_t data_low=0,data_high=0;
   u_int32_t event_length = (u_int32_t)input_nof_samples;

   bank1_armed_flag = 0; // *armed_bank_flag;  // keeping track of previous bank.  armed_bank_flag: 0 => bank2 armed; 1 => bank1 armed   

   if(isDebug) std::cout << "[SIS3316::ReadOutData]: Starting the readout loop..." << std::endl;
   if(EventNum==1){
      if(isDebug){
         sprintf(msg,"[SIS3316::ReadOutData]: THIS IS THE FIRST EVENT.  STARTING ON BANK 1");
         std::cout << msg << std::endl;
         sprintf(msg,"[SIS3316::ReadOutData]: SIS3316_KEY_DISARM_AND_ARM_BANK1");
         std::cout << msg << std::endl;
      }
      // only do this on the first event to get things rolling 
      addr = base_addr + SIS3316_KEY_DISARM_AND_ARM_BANK1; 
      rc = CommDriver::vme_write32(vme_handle,addr,0x0);  //  Arm Bank1
      bank1_armed_flag = 1; // start condition
   }

   if(isDebug){
      sprintf(msg,"[SIS3316::ReadOutData]: bank1_armed_flag = %d",bank1_armed_flag);
      std::cout << msg << std::endl;
      sprintf(msg,"[SIS3316::ReadOutData]: EVENT NUMBER: %d",EventNum);
      std::cout << msg << std::endl;
      sprintf(msg,"[SIS3316::ReadOutData]: [START] bank1_armed_flag = %d ",bank1_armed_flag);
      std::cout << msg << std::endl;
   }
  
   addr = base_addr + SIS3316_KEY_TRIGGER;
   rc = CommDriver::vme_write32(vme_handle,addr,0x0);
   poll_counter = 0 ;

   do{
      poll_counter++;
      if (poll_counter==poll_counter_max){
         // if(isDebug) printf("[SIS3316::ReadOutData]: Address threshold has NOT been reached yet... this is taking longer than expected.  Exiting. \n"); 
         if(isDebug){
            sprintf(msg,"[SIS3316::ReadOutData]: Address threshold has NOT been reached yet... this is taking longer than expected.  Exiting.");
            std::cout << msg << std::endl;
         }
         addr = base_addr + SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG; 
         rc = CommDriver::vme_read32(vme_handle,addr,&addr_thresh);
         if(isDebug){
            sprintf(msg,"[SIS3316::ReadOutData]: address thresh  = %lu \n",(unsigned long int)addr_thresh);
            std::cout << msg << std::endl;
            sprintf(msg,"[SIS3316::ReadOutData]: acq. ctrl. data: 0x%08x",read_data);
            std::cout << msg << std::endl;
         }
         addr = base_addr + SIS3316_ADC_CH1_ACTUAL_SAMPLE_ADDRESS_REG; 
         rc = CommDriver::vme_read32(vme_handle,addr,&read_data_2);
         if(isDebug){
            sprintf(msg,"[SIS3316::ReadOutData]: actual sample address register: 0x%08x \n",read_data_2);
            std::cout << msg << std::endl;
         }
         return -98;
      }
      addr = base_addr + SIS3316_ACQUISITION_CONTROL_STATUS; 
      rc = CommDriver::vme_read32(vme_handle,addr,&read_data);
      // usleep(500000); //500ms
      usleep(1);
   } while ( (read_data & 0x80000)==0x0 ); // has the Address Threshold been reached? If 0, then address threshold has NOT been reached.

   addr = base_addr + SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG; 
   rc = CommDriver::vme_read32(vme_handle,addr,&addr_thresh);

   if(isDebug){
      printf("[SIS3316::ReadOutData]: address thresh  = %lu \n",(unsigned long int)addr_thresh);
      std::cout << msg << std::endl;
      sprintf(msg,"[SIS3316::ReadOutData]: Address threshold reached!  Switching banks... \n");
      std::cout << msg << std::endl;
      sprintf(msg,"[SIS3316::ReadOutData]: ACQUISITION CONTROL STATUS: 0x%08x\n", read_data);
      std::cout << msg << std::endl;
   }

   // get ready for next event: disarm current bank and arm the next bank 
   if(bank1_armed_flag == 1){
      addr = base_addr + SIS3316_KEY_DISARM_AND_ARM_BANK2; 
      rc = CommDriver::vme_write32(vme_handle,addr,0x0);  //  Arm Bank2
      bank1_armed_flag = 0; // bank 2 is armed
      if(isDebug){
         sprintf(msg,"[SIS3316::ReadOutData]: SIS3316_KEY_DISARM_AND_ARM_BANK2");
         std::cout << msg << std::endl;
      }
   }else{
      addr = base_addr + SIS3316_KEY_DISARM_AND_ARM_BANK1; 
      rc = CommDriver::vme_write32(vme_handle,addr,0x0);  //  Arm Bank1
      bank1_armed_flag = 1; // bank 1 is armed
      if(isDebug){
         sprintf(msg,"[SIS3316::ReadOutData]: SIS3316_KEY_DISARM_AND_ARM_BANK1");
         std::cout << msg << std::endl;
      }
   }

   if(isDebug){
      sprintf(msg,"[SIS3316::ReadOutData]: [END] bank1_armed_flag = %d",bank1_armed_flag);
      std::cout << msg << std::endl;
   }

   // bookkeeping of armed bank
   // *armed_bank_flag = bank1_armed_flag;

   if(isDebug){
      sprintf(msg,"[SIS3316::ReadOutData]: BANK1 FLAG IS NOW %d",bank1_armed_flag);
      std::cout << msg << std::endl;
   }

   usleep(10000);  // wait 10 ms 

   // Read out data
   for(int ch=start_ch;ch<end_ch;ch++){
      if(isDebug){
         sprintf(msg,"[SIS3316::ReadOutData]: Reading channel %d",ch+1);
         std::cout << msg << std::endl;
      }
      rc = read_DMA_Channel_PreviousBankDataBuffer(vme_handle,
                                                         bank1_armed_flag,
                                                         ch,
                                                         max_read_nof_words,
                                                         &got_nof_32bit_words,
                                                         adc_buffer,
                                                         event_length);
      if(isDebug){
         sprintf(msg,"[SIS3316::ReadOutData]: read_DMA_Channel_PreviousBankDataBuffer: ");
         std::cout << msg;
         sprintf(msg,"ch = %d  got_nof_32bit_words = 0x%08x (%d) return_code = 0x%08x",
                 ch+1,got_nof_32bit_words,got_nof_32bit_words,rc);
         std::cout << msg << std::endl;
      }
   }

   if(rc!=0x900){
      if(got_nof_32bit_words>0){
         for(i=0;i<got_nof_32bit_words;i++){
            data_low     =  adc_buffer[i] & 0x0000ffff;
            data_high    = (adc_buffer[i] & 0xffff0000)/pow(2,16);
            fData[i*2]   = (u_int16_t)data_low;
            fData[i*2+1] = (u_int16_t)data_high;
         }
         if(isDebug){
            sprintf(msg,"[SIS3316::ReadOutData]: Event %d: Recorded %d 32-bit data-words.",EventNum,got_nof_32bit_words);
            std::cout << msg << std::endl;
         }
      }else{
         sprintf(msg,"[SIS3316::ReadOutData]: No 32-bit words found! No data recorded! Moving on...");
         std::cout << msg << std::endl;
         rc = -97;
      }
   }else{
      sprintf(msg,"[SIS3316::ReadOutData]: read_DMA_Channel_PreviousBankDataBuffer return code 0x900.  No data recorded! Moving on...");
      std::cout << msg << std::endl;
      rc = -97;
   }
   // printf("---------------------------------------------------- \n");

   if(isDebug){
      sprintf(msg,"[SIS3316::SIS3316SampleData]: Return code = %d  ",rc);
      std::cout << msg << std::endl;
      sprintf(msg,"[SIS3316::SIS3316SampleData]: event length = %lu",(unsigned long int)event_length);
      std::cout << msg << std::endl;
   }

   return rc;
}
