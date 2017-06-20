// test the keithley comms 

#include <iostream>
#include <fstream>
#include <string>

#include "KeithleyInterface.hh"

int main(){
  
   std::string dev_name = "KEITHLEY"; 
   std::string dev_path = "/dev/usbtmc"; 
  
   int protocol = comm_driver::kUSBTMC; 
 
   int portNo = keithley_interface::open_connection( protocol,dev_name.c_str(),dev_path.c_str() ); 
   int rc     = keithley_interface::close_connection(protocol,portNo);

   return rc;
}
