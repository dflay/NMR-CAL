#include "SG382.hh"
//______________________________________________________________________________
SG382::SG382(){
   fName = "SRS SG382"; 
}
//______________________________________________________________________________
SG382::~SG382(){

}
//______________________________________________________________________________
int SG382::SetFrequency(const char *freq){
   // string is of the form: "value units" 
   std::cout << fHandle << std::endl; 
   char cmd[100];
   sprintf(cmd, "FREQ %s\n",freq);
   int rc = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   return rc;
}
//______________________________________________________________________________
int SG382::SetNTypeAmplitude(const char *amp){
   // string is of the form: "value units" 
   char cmd[100];
   sprintf(cmd, "AMPR %s\n",amp);
   int rc = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   return rc;
}
//______________________________________________________________________________
int SG382::SetBNCAmplitude(const char *amp){
   // string is of the form: "value units" 
   char cmd[100];
   sprintf(cmd, "AMPL %s\n",amp);
   int rc = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   return rc;
}
//______________________________________________________________________________
int SG382::SetBNCOutputState(int flag){
   int rc=-1;
   char cmd[100];
   if(flag==sg382_interface::kDISABLE || flag==sg382_interface::kENABLE){
      sprintf(cmd,"ENBL%d\n",flag);
      rc = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   }else{
      printf("[SG382::SetBNCOutputState]: ERROR: Invalid flag '%d' passed.\n",flag);
   }
   return rc;
}
//______________________________________________________________________________
int SG382::SetNTypeOutputState(int flag){
   int rc=-1;
   char cmd[100];
   if(flag==sg382_interface::kDISABLE || flag==sg382_interface::kENABLE){
      sprintf(cmd,"ENBR%d\n",flag);
      rc = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   }else{
      printf("[SG382::SetBNCOutputState]: ERROR: Invalid flag '%d' passed.\n",flag);
   }
   return rc;
}
//______________________________________________________________________________
int SG382::SetModulationState(int flag){
   int rc=-1;
   char cmd[100];
   if(flag==sg382_interface::kDISABLE || flag==sg382_interface::kENABLE){
      sprintf(cmd,"MODL%d\n",flag);
      rc = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   }else{
      printf("[SG382::SetModulationState]: ERROR: Invalid flag '%d' passed.\n",flag);
   }
   return rc;
}
//______________________________________________________________________________
int SG382::SetModulationFunction(int flag){
   int rc=-1;
   char cmd[100];
   if(flag==sg382_interface::kSINE   || flag==sg382_interface::kRAMP  || flag==sg382_interface::kTRIANGLE ||
      flag==sg382_interface::kSQUARE || flag==sg382_interface::kNOISE || flag==sg382_interface::kEXTERNAL){
      sprintf(cmd,"MFNC%d\n",flag);
      rc = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   }else{
      printf("[SG382::SetModulationFunction]: ERROR: Invalid flag passed.\n");
   }
   return rc;
}
//______________________________________________________________________________
int SG382::SetModulationRate(double freq){
   char cmd[100];
   sprintf(cmd,"RATE %.14lf\n",freq);
   int rc = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   return rc;
}
//______________________________________________________________________________
int SG382::GetBNCOutputState(int &state){
   char cmd[100],response[100];
   sprintf(cmd,"ENBL?\n");
   int rc = CommDriver::query(fProtocol,fHandle,cmd,response);
   state = atoi(response);
   return rc;
}
//______________________________________________________________________________
int SG382::GetNTypeOutputState(int &state){
   char cmd[100],response[100];
   sprintf(cmd,"ENBR?\n");
   int rc = CommDriver::query(fProtocol,fHandle,cmd,response);
   state = atoi(response);
   return rc;
}
//______________________________________________________________________________
int SG382::GetBNCAmplitude(double &amp){
   char cmd[100],response[100];
   sprintf(cmd,"AMPL?\n");
   int rc = CommDriver::query(fProtocol,fHandle,cmd,response);
   amp = atof(response);
   return rc;
}
//______________________________________________________________________________
int SG382::GetNTypeAmplitude(double &amp){
   char cmd[100],response[100];
   sprintf(cmd,"AMPR?\n");
   int rc = CommDriver::query(fProtocol,fHandle,cmd,response);
   amp = atof(response);
   return rc;
}
//______________________________________________________________________________
int SG382::GetFrequency(double &freq){
   char cmd[100],response[100];
   sprintf(cmd,"FREQ?\n");
   int rc = CommDriver::query(fProtocol,fHandle,cmd,response);
   freq = atof(response);
   return rc;
}
//______________________________________________________________________________
int SG382::CheckErrors(char *err_msg){
   const int SIZE = 512;
   char cmd[SIZE],response[SIZE];
   sprintf(cmd,"LERR?\n");
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

