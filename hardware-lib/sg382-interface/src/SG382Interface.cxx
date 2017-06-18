#include "SG382Interface.hh"

namespace sg382_interface { 
   //______________________________________________________________________________
   int open_connection(int type,const char *device_path){
      int handle = -1; 
      switch (type) { 
         case comm_driver::kRS232:
            handle = comm_driver::rs232_open_connection(device_path); 
            break;
         case comm_driver::kUSBTMC: 
            break;
         case comm_driver::kTCPIP: 
            break;
         default:
            std::cout << "[sg382_interface::open_connection]: Invalid protocol!" << std::endl;
      }
      return handle;
   }
   //______________________________________________________________________________
   int close_connection(int type,int handle){
      int rc=-1; 
      switch (type) { 
         case comm_driver::kRS232:
            rc = comm_driver::rs232_close_connection(handle); 
            break;
         case comm_driver::kUSBTMC: 
            break;
         case comm_driver::kTCPIP: 
            break;
         default:
            std::cout << "[sg382_interface::close_connection]: Invalid protocol!" << std::endl;
      }
      return rc;
   }
   //______________________________________________________________________________
   int write_cmd(int type,int handle,const char *buffer){
      int rc=0;
       switch (type) { 
         case comm_driver::kRS232:
            rc = comm_driver::rs232_write(handle,buffer); 
            break;
         case comm_driver::kUSBTMC: 
            break;
         case comm_driver::kTCPIP: 
            break;
         default:
            std::cout << "[sg382_interface::write_cmd]: Invalid protocol!" << std::endl;
      }
      return rc;     
   }
   //______________________________________________________________________________
   int ask(int type,int handle,const char *query,char *response){
      int rc=0;
       switch (type) { 
         case comm_driver::kRS232:
            rc = comm_driver::rs232_ask(handle,query,response); 
            break;
         case comm_driver::kUSBTMC: 
            break;
         case comm_driver::kTCPIP: 
            break;
         default:
            std::cout << "[sg382_interface::ask]: Invalid protocol!" << std::endl;
      }
      return rc;
   }
   //______________________________________________________________________________
   int clear_error(int type,int handle){
      const int SIZE = 100; 
      char cmd[SIZE];
      sprintf(cmd,"*CLS\n");
      int rc = write_cmd(type,handle,cmd); 
      return rc; 
   }
   //______________________________________________________________________________
   int set_freq(int type,int handle,double freq) {
      char freq_str[100];
      sprintf(freq_str, "FREQ %.7lf\n",freq);
      return write_cmd(type,handle,freq_str);
   }
   //______________________________________________________________________________
   int set_bnc_amp(int type,int handle,double amp) {
      char amp_str[100];
      sprintf(amp_str, "AMPL %.7lf\n",amp);
      return write_cmd(type,handle,amp_str);
   }
   //______________________________________________________________________________
   int set_ntype_amp(int type,int handle,double amp) {
      char amp_str[100];
      sprintf(amp_str, "AMPR %.7lf\n",amp);
      return write_cmd(type,handle,amp_str);
   }
   //______________________________________________________________________________
   int set_bnc_output_state(int type,int handle,int flag) {
      int rc=-1;
      char cmd[100]; 
      switch (flag) {
	 case kDISABLE:
            sprintf(cmd,"ENBL0\n");  
	    break;
	 case kENABLE:
            sprintf(cmd,"ENBL1\n");  
	    break;
	 default:
	    printf("[sg382_interface::set_bnc_output]: ERROR: Invalid flag passed.\n");
      }
      rc = write_cmd(type,handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_ntype_output_state(int type,int handle,int flag) {
      int rc=-1;
      char cmd[100]; 
      switch (flag) {
	 case kDISABLE:
            sprintf(cmd,"ENBR0\n"); 
	    break;
	 case kENABLE:
            sprintf(cmd,"ENBR0\n"); 
	    break;
	 default:
	    printf("[sg382_interface::set_ntype_output]: ERROR: Invalid flag passed.\n");
      }
      rc = write_cmd(type,handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_modulation_state(int type,int handle,int flag) {
      int rc=-1;
      char cmd[100]; 
      switch (flag) {
	 case kDISABLE:  
            sprintf(cmd,"MODL0\n");  
	    break;
	 case kENABLE:   
            sprintf(cmd,"MODL1\n");  
	    break;
	 default:
	    printf("[sg382_interface::set_modulation]: ERROR: Invalid flag passed.\n");
      }
      rc = write_cmd(type,handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_modulation_function(int type,int handle, int flag) {
      int rc=-1;
      char cmd[100]; 
      switch (flag) {
	 case kSINE:  
            sprintf(cmd,"MFNC0\n");  
	    break;
	 case kRAMP:   
            sprintf(cmd,"MFNC1\n");  
	    break;
	 case kTRIANGLE:   
            sprintf(cmd,"MFNC2\n");  
	    break;
	 case kSQUARE:   
            sprintf(cmd,"MFNC3\n");  
	    break;
	 case kNOISE:  
            sprintf(cmd,"MFNC4\n");  
	    break;
	 case kEXTERNAL:   
            sprintf(cmd,"MFNC5\n");  
	    break;
	 default:
	    printf("[sg382_interface::set_modulation_function]: ERROR: Invalid flag passed.\n");
      }
      rc = write_cmd(type,handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_modulation_rate(int type,int handle,double freq) {
      int rc = 0;
      char freq_str[100];
      sprintf(freq_str, "RATE %.14lf\n", freq);
      rc = write_cmd(type,handle,freq_str); 
      return rc;
   }
   //______________________________________________________________________________
   int get_bnc_output_state(int type,int handle,int &state){
      char query[100],response[100];
      sprintf(query,"ENBL?\n");
      int rc = ask(type,handle,query,response); 
      state = atoi(response); 
      return rc;  
   }
   //______________________________________________________________________________
   int get_ntype_output_state(int type,int handle,int &state){
      char query[100],response[100];
      sprintf(query,"ENBR?\n");
      int rc = ask(type,handle,query,response); 
      state = atoi(response); 
      return rc;  
   }
   //______________________________________________________________________________
   int get_bnc_amplitude(int type,int handle,double &amp){
      char query[100],response[100];
      sprintf(query,"AMPL?\n");
      int rc = ask(type,handle,query,response); 
      amp = atof(response); 
      return rc;  
   }
   //______________________________________________________________________________
   int get_ntype_amplitude(int type,int handle,double &amp){
      char query[100],response[100];
      sprintf(query,"AMPR?\n");
      int rc = ask(type,handle,query,response); 
      amp = atof(response); 
      return rc;  
   }
   //______________________________________________________________________________
   int get_frequency(int type,int handle,double &freq){
      char query[100],response[100];
      sprintf(query,"FREQ?\n");
      int rc = ask(type,handle,query,response); 
      freq = atof(response); 
      return rc;  
   }
   //______________________________________________________________________________
   int get_error(int type,int handle,char *response){
      const int SIZE = 512; 
      char query[SIZE];
      sprintf(query,"LERR?\n"); 
      int rc       = ask(type,handle,query,response); 
      int err_code = atoi(response); 
      rc *= 1; 
      return err_code;  
   }
}
