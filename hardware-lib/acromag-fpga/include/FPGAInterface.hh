#ifndef FPGA_HH
#define FPGA_HH

// a class to interface with the Acromag IP-EP201 FPGA
// FIXME: How to connect this back to the Device ABC?  

#include <cstdlib> 
#include <string>
#include <vector> 

#include "fpga_pulse_sequence.hh"
#include "fpga_addresses.hh"

class FPGA { 

   private:
      u_int16_t fCarrierAddress;
      u_int16_t fIOSpaceAddress;
      u_int16_t fIDSpaceAddress;

      bool fGlobalEnable;

      std::vector<fpga_pulse_sequence_t> fFPGAPulseSequence; 

   public:  
      FPGA();
      ~FPGA();

      int Init(); 

}; 

#endif 
