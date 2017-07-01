#include "KeithleyClass.hh"
//______________________________________________________________________________
Keithley::Keithley(){

}
//______________________________________________________________________________
Keithley::~Keithley(){

}
//______________________________________________________________________________
int Keithley::SetToRemoteMode(){
   const int SIZE = 512;
   char cmd[SIZE];
   sprintf(cmd,"SYST:REM\n");
   int rc   = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   return rc;
}
//______________________________________________________________________________
int Keithley::SetRange(double maxRange){
   const int SIZE = 512;
   char cmd[SIZE];
   sprintf(cmd,"CONF:RES %.3lf\n",maxRange);
   int rc   = CommDriver::write_cmd(fProtocol,fHandle,cmd);
   return rc;
}
//______________________________________________________________________________
int Keithley::GetResistance(double &R){
   const int SIZE = 512;
   char query[SIZE],response[SIZE];
   sprintf(query,"MEAS:RES?\n");
   int rc = CommDriver::query(fProtocol,fHandle,query,response);
   R      = atof(response);
   return rc;
}
