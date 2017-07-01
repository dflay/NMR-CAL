#ifndef DEVICE_HH
#define DEVICE_HH

// an abstract base class for a device 

#include <cstdlib>
#include <iostream> 
#include <fstream>
#include <string>  

#include "CommDriver.hh"

class Device{ 

   public:
      int OpenConnection(); 
      int CloseConnection();
      
      virtual int GetDeviceID(char* response); 
      virtual int CheckErrors(char* err_msg);
      virtual int ClearErrors();
 
      void SetProtocol(int p)        { fProtocol = p;    } 
      void SetName(const char* name) { fName     = name; } 
      void SetPath(const char* path) { fPath     = path; } 

      int GetProtocol()        const { return fProtocol; } 
      int GetHandle()          const { return fHandle;   }

      std::string GetName()    const { return fName;     }  
      std::string GetPath()    const { return fPath;     }  
 
      Device();
      virtual ~Device();
  
   protected: 
      int fProtocol;         // type of protocol (RS232, USBTMC, TCPIP) 
      int fHandle;           // port number returned when connecting 
      std::string fName;     // device name 
      std::string fPath;     // device path 

}; 

#endif 
