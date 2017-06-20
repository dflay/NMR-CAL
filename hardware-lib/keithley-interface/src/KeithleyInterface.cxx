#include "KeithleyInterface.hh"

namespace keithley_interface {
   //______________________________________________________________________________
   int open_connection(int type,const char *dev_name,const char *dev_path){
      int handle = comm_driver::open_connection(type,dev_name,dev_path); 
      return handle; 
   }
   //______________________________________________________________________________
   int close_connection(int type,int handle){
      int rc = comm_driver::close_connection(type,handle);
      return rc; 
   }
   //______________________________________________________________________________
   int get_device_id(int type,int portNo,char *response){
      const int SIZE = 512;
      char query[SIZE];
      sprintf(query,"*IDN?\n"); 
      int rc = comm_driver::query(type,portNo,query,response);
      return rc; 
   }
   //______________________________________________________________________________
   int get_mode(int type,int portNo,char *response){
      const int SIZE = 512; 
      char query[SIZE]; 
      sprintf(query,"SENS:FUNC?\n"); 
      int rc = comm_driver::query(type,portNo,query,response);
      return rc;   
   }
   //______________________________________________________________________________
   int check_errors(int type,int portNo,char *err_msg){
      const int SIZE = 512; 
      char query[SIZE]; 
      sprintf(query,"SYST:ERR?\n");
      int rc = comm_driver::query(type,portNo,query,err_msg);
      // FIXME: parse the string; it's going to be an error code and a message
      printf("keithley error message: %s \n",err_msg); 
      return rc;
   }
   //______________________________________________________________________________
   int set_to_remote_mode(int type,int portNo){
      const int SIZE = 512;
      char cmd[SIZE];
      sprintf(cmd,"SYST:REM\n"); 
      int rc   = comm_driver::write_cmd(type,portNo,cmd);  
      return rc;
   }
   //______________________________________________________________________________
   int set_range(int type,int portNo,double maxRange){
      const int SIZE = 512;
      char cmd[SIZE];
      sprintf(cmd,"CONF:RES %.3lf\n",maxRange); 
      int rc   = comm_driver::write_cmd(type,portNo,cmd);  
      return rc;
   }
   //______________________________________________________________________________
   int get_resistance(int type,int portNo,double &R){
      const int SIZE = 512;
      char query[SIZE],response[SIZE];
      sprintf(query,"MEAS:RES?\n"); 
      int rc = comm_driver::query(type,portNo,query,response);  
      R      = atof(response); 
      return rc; 
   }

}  // ::keithley_interface 

