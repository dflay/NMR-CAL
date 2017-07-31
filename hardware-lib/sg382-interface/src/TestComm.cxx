// test the SG382 comms  

#include <cstdlib> 
#include <iostream>
#include <fstream>
#include <string>

// #include "SG382Interface.hh"
#include "SG382.hh"

int main(){

   // int protocol         = comm_driver::kRS232;  
   // std::string dev_addr = "/dev/ttyUSB1";
   // std::string dev_name = "SG382"; 
 
   // int handle = sg382_interface::open_connection( protocol,dev_name.c_str(),dev_addr.c_str() ); 
   // int rc     = sg382_interface::close_connection(protocol,handle); 

   SG382 *mySG382 = new SG382();
   mySG382->SetProtocol(CommDriver::kRS232); 
   mySG382->SetName("SRS SG382");
   mySG382->SetPath("/dev/ttyUSB1"); 
   mySG382->OpenConnection();
   mySG382->CloseConnection(); 

   delete mySG382;  

   return 0;
}
