#include "FPGAInterface.hh"
//______________________________________________________________________________
FPGA::FPGA(){

}
//______________________________________________________________________________
FPGA::~FPGA(){

}
//______________________________________________________________________________
int FPGA::Init(){
   fGlobalEnable   = false; 
   fCarrierAddress = 0;
   fIOSpaceAddress = 0;
   fIDSpaceAddress = 0;
   fFPGAPulseSequence.clear();
   return 0; 
}

