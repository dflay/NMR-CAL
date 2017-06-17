#ifndef KEITHLEY_INTERFACE_HH
#define KEITHLEY_INTERFACE_HH

// functions for comms with the Keithley DMM

#include <iostream>
#include <fstream>
#include <sstream>
#include <string> 
#include <fcntl.h>    // For O_RDWR 
#include <unistd.h>   // For open(), creat()  

namespace keithley_interface {
 
   int open_connection(const char *dev_path);
   int close_connection(int portNo);
   int get_device_id(int portNo,char *response); 
   int get_mode(int portNo,char *response); 
   int set_to_remote_mode(int portNo); 
   int set_range(int portNo,double maxRange); 
   int check_errors(int portNo,char *err_msg); 
   int write_cmd(int portNo,const char *cmd); 
   int ask(int portNo,const char *query,char *response); 

   double get_resistance(int portNo);

}

#endif 
