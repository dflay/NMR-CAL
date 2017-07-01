// test the keithley comms 

#include <iostream>
#include <fstream>
#include <string>

#include "KeithleyInterface.hh"
#include "KeithleyClass.hh"

int main(){

   std::string dev_name = "KEITHLEY"; 
   std::string dev_path = "/dev/usbtmc"; 

   // first approach   
  
   // int protocol = CommDriver::kUSBTMC; 
 
   // int portNo = keithley_interface::open_connection( protocol,dev_name.c_str(),dev_path.c_str() ); 

   // int rc = keithley_interface::set_to_remote_mode(protocol,portNo); 

   // char msg[512];
   // rc = keithley_interface::get_device_id(protocol,portNo,msg); 
   // std::cout << msg << std::endl;  

   // double maxRange = 100E+3; 
   // rc = keithley_interface::set_range(protocol,portNo,maxRange);

   // double R=0;
   // rc = keithley_interface::get_resistance(protocol,portNo,R);
   // std::cout << "Resistance = " << R << std::endl; 
 
   // strcpy(msg,""); 
   // rc = keithley_interface::check_errors(protocol,portNo,msg); 
   // std::cout << "Error code " << rc << ": " << msg << std::endl;  
   // strcpy(msg,""); 

   // rc = keithley_interface::clear_errors(protocol,portNo); 
   // rc = keithley_interface::check_errors(protocol,portNo,msg); 
   // std::cout << "Error code " << rc << ": " << msg << std::endl;  
 
   // rc = keithley_interface::close_connection(protocol,portNo);

   // second approach 
   Keithley *myKeithley = new Keithley(); 
   myKeithley->SetProtocol(CommDriver::kUSBTMC); 
   myKeithley->SetName( dev_name.c_str() ); 
   myKeithley->SetPath( dev_path.c_str() );

   int rc = myKeithley->OpenConnection(); 
   if (rc!=0) {
      std::cout << "Cannot open connection!" << std::endl;
      return 1; 
   } 

   rc = myKeithley->SetToRemoteMode(); 

   char msg[512];
   rc = myKeithley->GetDeviceID(msg); 
   std::cout << msg << std::endl;  

   double maxRange = 100E+3; 
   rc = myKeithley->SetRange(maxRange);

   double R=0;
   rc = myKeithley->GetResistance(R);
   std::cout << "Resistance = " << R << std::endl; 
 
   strcpy(msg,""); 
   rc = myKeithley->CheckErrors(msg); 
   std::cout << "Error code " << rc << ": " << msg << std::endl;  
   strcpy(msg,""); 

   if(rc!=0){
      rc = myKeithley->ClearErrors();
      strcpy(msg,""); 
      rc = myKeithley->CheckErrors(msg); 
      std::cout << "Error code " << rc << ": " << msg << std::endl;  

   } 

   rc = myKeithley->CloseConnection(); 

   delete myKeithley; 

   return rc;
}
