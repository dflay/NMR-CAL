#ifndef FPGA_ADDRESSES_HH
#define FPGA_ADDRESSES_HH

#define FPGA_CLOCK_FREQ          8E+6        // 8 MHz clock 

#define MODULE_BASE_ADDR         0x00000000

#define IP_A_IO_SPACE_ADDR       0x00000000
#define IP_B_IO_SPACE_ADDR       0x00000100
#define IP_C_IO_SPACE_ADDR       0x00000200
#define IP_D_IO_SPACE_ADDR       0x00000300

#define IO_SPACE_OFFSET          0x80 

#define FLAG_ADDR                0x0062
#define NEW_FLAG_ADDR            0x0028      // NOTE: This is NOT the same memory space as SRAM! 
#define MECHANICAL_SWITCH_ADDR   0x0002 
#define RF_SWITCH_TRANS_ADDR     0x000a
#define RF_SWITCH_REC_ADDR       0x0012
#define TOMCO_ADDR               0x001a
#define DIGITIZER_ADDR_1         0x0020
#define DIGITIZER_ADDR_2         0x0024
#define UPDATE_ADDR              0x0054
#define COUNTER_ENABLE_ADDR      0x0056

#define MECH_SWITCH_NAME         "mech_sw"

#define GLOBAL_ON_OFF_NAME       "global_on_off"
#define MECH_SWITCH_1_NAME       "mech_sw_1"        
#define MECH_SWITCH_2_NAME       "mech_sw_2"        
#define MECH_SWITCH_3_NAME       "mech_sw_3"        
#define MECH_SWITCH_4_NAME       "mech_sw_4"        
#define RF_TRANSMIT_NAME         "rf_trans"
#define RF_RECEIVE_NAME          "rf_rec"
#define TOMCO_NAME               "tomco"

#endif 
