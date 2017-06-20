// test the SG382 comms  

#include <cstdlib> 
#include <iostream>
#include <fstream>
#include <string>

#include "SG382Interface.hh"

int main(){

   int protocol         = comm_driver::kRS232;  
   std::string dev_addr = "/dev/ttyUSB1";
   std::string dev_name = "SG382"; 
 
   int handle = sg382_interface::open_connection( protocol,dev_name.c_str(),dev_addr.c_str() ); 
   int rc     = sg382_interface::close_connection(protocol,handle); 

   return rc;
}
