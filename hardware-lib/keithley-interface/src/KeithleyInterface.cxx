#include "KeithleyInterface.hh"

namespace keithley_interface {
   //______________________________________________________________________________
   int open_connection(const char *dev_path){
      int SIZE = 128;
      char DEV_PATH[SIZE]; 
      sprintf(DEV_PATH,"%s0",dev_path);

      // FIXME: is there a better way to do this?  
      std::stringstream ss;
      std::string s;
      std::ifstream in(DEV_PATH);

      // look for the keithley 
      if( in.good() ){
	 ss << in.rdbuf();
	 in.close();
      }else{
	 std::cout << "No USBTMC devices found" << std::endl;
	 return 1;
      }

      char devName[SIZE];
      char mfg[] = "KEITHLEY"; 

      for(int i=0;i<16;i++){
	 std::getline(ss, s, '\n');
	 if(s.find(mfg) != s.size() - 1){
	    std::cout << s << std::endl;
	    sprintf(devName, "/dev/usbtmc%i", i);
	 }
      }

      int portNo=-1; 
      portNo = open(devName,O_RDWR);
      return portNo;   
   }
   //______________________________________________________________________________
   int close_connection(int portNo){
      int rc = close(portNo);
      return rc; 
   }
   //______________________________________________________________________________
   int get_device_id(int portNo,char *response){
      const int SIZE = 512;
      char query[SIZE];
      sprintf(query,"*IDN?\n"); 
      int rc = ask(portNo,query,response);
      return rc; 
   }
   //______________________________________________________________________________
   int get_mode(int portNo,char *response){
      const int SIZE = 512; 
      char query[SIZE]; 
      sprintf(query,"SENS:FUNC?\n"); 
      int rc = ask(portNo,query,response);
      return rc;   
   }
   //______________________________________________________________________________
   int check_errors(int portNo,char *err_msg){
      const int SIZE = 512; 
      char query[SIZE],response[SIZE]; 
      sprintf(query,"SYST:ERR?\n");
      int rc = ask(portNo,query,err_msg);
      // FIXME: parse the string; it's going to be an error code and a message
      printf("keithley error message: %s \n",err_msg); 
      return rc;
   }
   //______________________________________________________________________________
   int set_to_remote_mode(int portNo){
      const int SIZE = 512;
      char cmd[SIZE];
      sprintf(cmd,"SYST:REM\n"); 
      int rc   = write_cmd(portNo,cmd);  
      return rc;
   }
   //______________________________________________________________________________
   int set_range(int portNo,double maxRange){
      const int SIZE = 512;
      char cmd[SIZE];
      sprintf(cmd,"CONF:RES %.3lf\n",maxRange); 
      int rc   = write_cmd(portNo,cmd);  
      return rc;
   }
   //______________________________________________________________________________
   double get_resistance(int portNo){
      const int SIZE = 512;
      char query[SIZE],response[SIZE];
      sprintf(query,"MEAS:RES?\n"); 
      int rc   = ask(portNo,query,response);  
      double R = atof(response); 
      return R; 
   }
   //______________________________________________________________________________
   int write_cmd(int portNo,const char *cmd){
      int rc = write( portNo,cmd,strlen(cmd) );  
      return rc; 
   }
   //______________________________________________________________________________
   int ask(int portNo,const char *query,char *response){
      int SIZE = 512; 
      int r = write_cmd(portNo,query);   
      int b = read(portNo,&response,SIZE);
      if(r!=0||b!=0) strcpy(response,"NO RESPONSE");    // comms failed   
      return b; 
   }

}  // ::keithley_interface 

