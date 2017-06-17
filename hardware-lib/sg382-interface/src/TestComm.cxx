// test the keithley comms 

#include <iostream>
#include <fstream>
#include <string>

#include "SG382Interface.hh"

int main(){
  
   std::string dev_path = "/dev/ttyUSB1"; 
   int rs232_handle     = sg382_interface::open_connection( dev_path.c_str() ); 
   int rc               = sg382_interface::close_connection(rs232_handle); 

   return 0;
}
