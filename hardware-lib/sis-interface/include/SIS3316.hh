#ifndef SIS3316_H
#define SIS3316_H 

// implementation of SIS3316 class

#include "SISBase.hh"
#include "sis3316_var.h"

class SIS3316: public SISBase { 

   public:
      SIS3316( sisParameters_t par=sisParameters() );
      ~SIS3316(); 

      int Initialize();
      int ReInitialize();
      int ReadOutData(std::vector<double> &outData);

   private: 

      int fEventNumber;
      int *fArmedBankFlag; 

      int call_vme_A32MBLT64FIFO_read(int vme_handle, u_int32_t vme_adr, u_int32_t* vme_data,
            u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords);

      int read_DMA_Channel_PreviousBankDataBuffer(int vme_handle,
            unsigned int bank2_read_flag,
            unsigned int channel_no,
            unsigned int max_read_nof_words,
            unsigned int *dma_got_no_of_words,
            u_int32_t *uint_adc_buffer,
            u_int32_t NumOfEvents);

      int configure_clock(int use_ext_clock,int adc_125MHz_flag);
      int I2cStart(int vme_handle,int osc);
      int I2cStop(int vme_handle,int osc);
      int I2cWriteByte(int vme_handle,int osc, unsigned char data, char *ack);
      int I2cReadByte(int vme_handle,int osc, unsigned char *data, char ack);
      int Si570ReadDivider(int vme_handle,int osc, unsigned char *data);
      int Si570Divider(int vme_handle,int osc, unsigned char *data);
      int Si570FreezeDCO(int vme_handle,int osc);
      int Si570UnfreezeDCO(int vme_handle,int osc);
      int Si570NewFreq(int vme_handle,int osc);
      int set_frequency(int vme_handle,int osc, unsigned char *values);
      int change_frequency_HSdiv_N1div(int vme_handle,int osc, unsigned hs_div_val, unsigned n1_div_val);
      int vme_write_si5325(int vme_handle,u_int32_t si5325_addr,u_int32_t data32);
      int adc_spi_setup(int vme_handle,int adc_125MHz_flag);
      int adc_spi_write(int vme_handle,unsigned int adc_fpga_group, unsigned int adc_chip, unsigned int spi_addr, unsigned int spi_data);
      int adc_spi_read(int vme_handle,unsigned int adc_fpga_group, unsigned int adc_chip, unsigned int spi_addr, unsigned int* spi_data); 

}; 

#endif 
