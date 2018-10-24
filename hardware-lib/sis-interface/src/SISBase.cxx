#include "SISBase.hh"
//______________________________________________________________________________
SISBase::SISBase(sisParameters_t par){
   fProtocol          = CommDriver::kVME;
   fName              = "SISBase"; 
   fPath              = "UNKNOWN";
   fHandle            = -1; 
   SetParameters(par);  
}
//______________________________________________________________________________
SISBase::~SISBase(){

}
//______________________________________________________________________________
void SISBase::SetParameters(sisParameters_t par){
   fParameters.moduleBaseAddress  = par.moduleBaseAddress;  
   fParameters.moduleID           = par.moduleID;  
   fParameters.channelNumber      = par.channelNumber;  
   fParameters.numberOfEvents     = par.numberOfEvents;  
   fParameters.clockType          = par.clockType;  
   fParameters.multiEventState    = par.multiEventState;  
   fParameters.debug              = par.debug; 
   // get the clock frequency in Hz 
   double sf=1.;
   if( par.clockFreqUnits==SISInterface::Hz ) sf = 1;
   if( par.clockFreqUnits==SISInterface::kHz) sf = 1E+3;
   if( par.clockFreqUnits==SISInterface::MHz) sf = 1E+6;
   if( par.clockFreqUnits==SISInterface::GHz) sf = 1E+9;
   fParameters.clockFrequency  = sf*par.clockFrequency;
   fParameters.clockPeriod     = 1./fParameters.clockFrequency;  
   // now the signal length in seconds  
   if( par.signalLengthUnits==SISInterface::nsec) sf = 1E-9;
   if( par.signalLengthUnits==SISInterface::usec) sf = 1E-6;
   if( par.signalLengthUnits==SISInterface::msec) sf = 1E-3;
   if( par.signalLengthUnits==SISInterface::sec ) sf = 1;
   fParameters.signalLength    = sf*par.signalLength;
   // derived terms  
   fParameters.numberOfSamples = fParameters.signalLength*fParameters.clockFrequency;  
}
//______________________________________________________________________________
int SISBase::Initialize(){
   return 0;
}
//______________________________________________________________________________
int SISBase::ReInitialize(){
   return 0;
}
//______________________________________________________________________________
int SISBase::ReadOutData(){
   return 0;
}
//______________________________________________________________________________
int SISBase::GetData(std::vector<unsigned short> &x) const{
   const int N = fData.size();
   if(N<=0){
      std::cout << "[SISBase::GetData]: No data in buffer!" << std::endl;
      return 1;
   }
   for(int i=0;i<N;i++) x.push_back(fData[i]); 
   return 0;
}
//______________________________________________________________________________
int SISBase::ReadModuleID(){
   // read the device ID data 
   // modID  = module ID 
   // majRev = major revision 
   // minRev = minor revision
   u_int32_t addr = fParameters.moduleBaseAddress + 0x4;  
   u_int32_t data=0;
   int modID=0;
   // int majRev=0,minRev=0;
   int rc    = CommDriver::vme_read32(fHandle,addr,&data);
   if(rc!=0){
      std::cout << "[SISBase::GetModuleID]:  Cannot read the module ID!" << std::endl;
      return 1;
   }else{
      modID =  data >> 16;
      fParameters.moduleID = modID; 
      // majRev    = (data >> 8) & 0xff;  
      // minRev    =  data & 0xff;  
   }
   return 0;
}
