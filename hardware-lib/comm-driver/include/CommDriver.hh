#ifndef COMM_DRIVER_HH
#define COMM_DRIVER_HH

// a library containing various communication protocols
// - RS232
// - USBTMC
// - TCPIP (Ethernet) 

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream> 
#include <unistd.h> 
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

namespace comm_driver {

   enum protocolType{
      kRS232  = 0,
      kUSBTMC = 1,
      kTCPIP  = 2
   };

   struct termios rs232_old_termios;

   int rs232_open_connection(const char *device_path);
   int rs232_close_connection(int rs232_handle); 
   int rs232_write(int handle,const char *cmd);
   int rs232_ask(int handle,const char *query,char *response); 

}

#endif 
