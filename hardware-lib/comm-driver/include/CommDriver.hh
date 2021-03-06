#ifndef COMM_DRIVER_HH
#define COMM_DRIVER_HH

// a library containing various communication protocols
// - RS232
// - USBTMC
// - TCPIP (Ethernet)
// - VME  

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream> 
#include <unistd.h> 
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "sis3100_vme_calls.h"

namespace CommDriver {

   enum protocolType{
      kRS232  = 0,
      kUSBTMC = 1,
      kTCPIP  = 2,
      kVME    = 3
   };

   // generic functions 
   int open_connection(int type,const char *device_name,const char *device_path); 
   int close_connection(int type,int handle); 
   int write_cmd(int type,int handle,const char *buffer);
   int query(int type,int handle,const char *cmd,char *response);
 
   // RS232 functions 
   int rs232_open_connection(const char *device_path);
   int rs232_close_connection(int rs232_handle); 
   int rs232_write(int handle,const char *cmd);
   int rs232_query(int handle,const char *cmd,char *response); 

   // USBTMC functions 
   int usbtmc_open_connection(const char *dev_name,const char *device_path); 
   int usbtmc_close_connection(int handle); 
   int usbtmc_write(int handle,const char *cmd);
   int usbtmc_query(int handle,const char *cmd,char *response);

   // VME functions 
   int vme_open_connection(const char *device_path);
   int vme_close_connection(int handle);  
   int vme_write32(int handle,u_int32_t addr,u_int32_t data);  
   int vme_read32(int handle,u_int32_t addr,u_int32_t *data);  

}

#endif 
