#include "Device.hh"
//______________________________________________________________________________
Device::Device(){
   fProtocol = -1;
   fHandle   = -1;  
}
//______________________________________________________________________________
Device::~Device(){

}
//______________________________________________________________________________
int Device::OpenConnection(){
   int rc=0;
   fHandle = CommDriver::open_connection(fProtocol,fName.c_str(),fPath.c_str() );
   if(fHandle<0) rc = -1; 
   return rc; 
}
//______________________________________________________________________________
int Device::CloseConnection(){
   int rc = CommDriver::close_connection(fProtocol,fHandle);
   return rc; 
}
//______________________________________________________________________________
int Device::GetDeviceID(char *response){
   const int SIZE = 512;
   char cmd[SIZE];
   sprintf(cmd,"*IDN?\n");
   int rc = CommDriver::query(fProtocol,fHandle,cmd,response);
   return rc; 
}
//______________________________________________________________________________
int Device::CheckErrors(char *err_msg){
   const int SIZE = 512;
   char cmd[SIZE],response[512];
   sprintf(cmd,"SYST:ERR?\n");
   int rc = CommDriver::query(fProtocol,fHandle,cmd,response);
   // parse the string; it's going to be an error code and a message
   std::istringstream ss(response);
   std::string token,entry[2];
   int i=0;
   while( std::getline(ss,token,',') ){
      entry[i] = token;
      i++;
   }
   // now return the data 
   rc = atoi(entry[0].c_str());         // zeroth entry is the error code
   strcpy( err_msg,entry[1].c_str() );  // copy the message to the buffer 
   return rc;
}
//______________________________________________________________________________
int Device::ClearErrors(){
   char cmd[20];
   sprintf(cmd,"*CLS");
   int rc = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   return rc;
}
