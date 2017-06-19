// test the SG382 comms  

#include <iostream>
#include <fstream>
#include <string>

#include "SG382Interface.hh"

int main(){

   int protocol = comm_driver::kRS232;  
   std::string dev_addr = "/dev/ttyUSB1"; 
   int handle = sg382_interface::open_connection( protocol,dev_addr.c_str() ); 
   int rc     = sg382_interface::close_connection(protocol,handle); 
   rc *= 1; 

   return 0;
}
