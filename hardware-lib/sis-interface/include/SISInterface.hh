#ifndef SIS_INTERFACE_H
#define SIS_INTERFACE_H 

// interface to talk to an SIS digitizer
// eventually break this into two classes SIS3302 and SIS3316 
// OR 
// Have a class called SISBase, and have specific models derive from it  

#include <cstdlib> 
#include <iostream>
#include <math.h>

#include "sisParameters.h"
#include "CommDriver.hh"

#include "sis1100_var.h"
#include "sis3302_var.h"
#include "sis3316_var.h"

#define MAX_NUMBER_LWORDS_64MBYTE 0x1000000       // 64MB 

namespace SISInterface { 

   extern bool isDebug; 

   int open_connection(int type,const char *device_name,const char *device_path);
   int close_connection(int type,int handle);
   int get_module_id(int handle,u_int32_t addr,int &modID,int &majRev,int &minRev);

   int initialize(sisParameters_t myADC);        // generic call the user interacts with  
   int initialize_3302(sisParameters_t myADC);   // for the 3302    
   int initialize_3316(sisParameters_t myADC);   // for the 3316
    
   int reinitialize(sisParameters_t myADC);        // generic call the user interacts with  
   int reinitialize_3302(sisParameters_t myADC);   // for the 3302    
   int reinitialize_3316(sisParameters_t myADC);   // for the 3316

   // functions specific to SIS3316 
   int configure_clock_3316(sisParameters_t myADC,int use_ext_clock,int adc_125MHz_flag);
   int call_vme_A32MBLT64FIFO_read(int vme_handle, u_int32_t vme_adr, u_int32_t* vme_data,
         u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords);

   int read_DMA_Channel_PreviousBankDataBuffer(int vme_handle,
         unsigned int bank2_read_flag,
         unsigned int channel_no,
         unsigned int max_read_nof_words,
         unsigned int *dma_got_no_of_words,
         u_int32_t *uint_adc_buffer,
         u_int32_t NumOfEvents);

   int I2cStart(int vme_handle,int osc);
   int I2cStop(int vme_handle,int osc);
   int I2cWriteByte(int vme_handle,int osc, unsigned char data, char *ack);
   int I2cReadByte(int vme_handle,int osc, unsigned char *data, char ack);
   int Si570FreezeDCO(int vme_handle,int osc);
   int Si570ReadDivider(int vme_handle,int osc, unsigned char *data);
   int Si570Divider(int vme_handle,int osc, unsigned char *data);
   int Si570UnfreezeDCO(int vme_handle,int osc);
   int Si570NewFreq(int vme_handle,int osc);
   int set_frequency(int vme_handle,int osc, unsigned char *values);
   int change_frequency_HSdiv_N1div(int vme_handle,int osc, unsigned hs_div_val, unsigned n1_div_val);
   int vme_write_si5325(int vme_handle,u_int32_t si5325_addr,u_int32_t data32); 
   int adc_spi_setup(int vme_handle,int adc_125MHz_flag);
   int adc_spi_write(int vme_handle,unsigned int adc_fpga_group, unsigned int adc_chip, unsigned int spi_addr, unsigned int spi_data);
   int adc_spi_read(int vme_handle,unsigned int adc_fpga_group, unsigned int adc_chip, unsigned int spi_addr, unsigned int* spi_data);

} //::SISInterface  

#endif 
