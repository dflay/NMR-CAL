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
   fParameters.clockFrequency     = par.clockFrequency;  
   fParameters.clockPeriod        = par.clockPeriod;  
   fParameters.signalLength       = par.signalLength; 
   fParameters.moduleID           = par.moduleID;  
   fParameters.channelNumber      = par.channelNumber;  
   fParameters.numberOfEvents     = par.numberOfEvents;  
   fParameters.numberOfSamples    = par.numberOfSamples;  
   fParameters.clockType          = par.clockType;  
   fParameters.multiEventState    = par.multiEventState;  
   fParameters.debug              = par.debug;  
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
int SISBase::GetModuleID(){
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
      return -1;
   }else{
      modID =  data >> 16;
      fParameters.moduleID = modID; 
      // majRev    = (data >> 8) & 0xff;  
      // minRev    =  data & 0xff;  
   }
   return fParameters.moduleID;
}
