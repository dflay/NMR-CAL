#ifndef FPGA_PULSE_SEQUENCE_HH
#define FPGA_PULSE_SEQUENCE_HH

// a data structure for an FPGA sequence 

#include <cstdlib> 
#include <string> 

typedef struct fpga_pulse_sequence { 
   // units for each type of timing 
   std::string sequence_name;              
   std::string mech_sw_units;         
   std::string rf_trans_units; 
   std::string rf_rec_units;
   std::string tomco_units; 
   // enable flags 
   int tomco_enable; 
   int enable_flag;
   // channel ID  
   int mech_sw_id;
   // start and stop times (low and high bytes)  
   int mech_sw_start_time_lo ,mech_sw_start_time_hi;   
   int mech_sw_end_time_lo   ,mech_sw_end_time_hi;   
   int rf_trans_start_time_lo,rf_trans_start_time_hi;   
   int rf_trans_end_time_lo  ,rf_trans_end_time_hi;   
   int rf_rec_start_time_lo  ,rf_rec_start_time_hi;   
   int rf_rec_end_time_lo    ,rf_rec_end_time_hi;   
   int tomco_start_time_lo   ,tomco_start_time_hi;   
   int tomco_end_time_lo     ,tomco_end_time_hi;   
} fpga_pulse_sequence_t; 

#endif 
