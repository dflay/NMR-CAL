// test the keithley comms 

#include <iostream>
#include <fstream>
#include <string>

#include "KeithleyInterface.hh"

int main(){
  
   std::string dev_path = "/dev/usbtmc"; 
   
   int portNo = keithley_interface::open_connection( dev_path.c_str() ); 
   int rc     = keithley_interface::close_connection(portNo); 

   return 0;
}
