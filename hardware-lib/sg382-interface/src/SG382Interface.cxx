#include "SG382Interface.hh"

namespace sg382_interface { 
   //______________________________________________________________________________
   int open_connection(int type,const char *device_path){
      int handle = comm_driver::open_connection(type,device_path); 
      return handle;
   }
   //______________________________________________________________________________
   int close_connection(int type,int handle){
      int rc = comm_driver::close_connection(type,handle); 
      return rc;
   }
   //______________________________________________________________________________
   int write_cmd(int type,int handle,const char *buffer){
      int rc = comm_driver::write_cmd(type,handle,buffer); 
      return rc;     
   }
   //______________________________________________________________________________
   int ask(int type,int handle,const char *query,char *response){
      int rc = comm_driver::query(type,handle,query,response);
      return rc; 
   }
   //______________________________________________________________________________
   int clear_error(int type,int handle){
      const int SIZE = 100; 
      char cmd[SIZE];
      sprintf(cmd,"*CLS\n");
      int rc = comm_driver::write_cmd(type,handle,cmd); 
      return rc; 
   }
   //______________________________________________________________________________
   int set_freq(int type,int handle,double freq) {
      char cmd[100];
      sprintf(cmd, "FREQ %.7lf\n",freq);
      int rc = comm_driver::write_cmd(type,handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_bnc_amp(int type,int handle,double amp) {
      char cmd[100];
      sprintf(cmd, "AMPL %.7lf\n",amp);
      int rc = comm_driver::write_cmd(type,handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_ntype_amp(int type,int handle,double amp) {
      char cmd[100];
      sprintf(cmd, "AMPR %.7lf\n",amp);
      int rc = comm_driver::write_cmd(type,handle,cmd); 
      return rc;
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
      rc = comm_driver::write_cmd(type,handle,cmd); 
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
      rc = comm_driver::write_cmd(type,handle,cmd); 
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
      rc = comm_driver::write_cmd(type,handle,cmd); 
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
      rc = comm_driver::write_cmd(type,handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_modulation_rate(int type,int handle,double freq) {
      char cmd[100];
      sprintf(cmd,"RATE %.14lf\n",freq);
      int rc = comm_driver::write_cmd(type,handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int get_bnc_output_state(int type,int handle,int &state){
      char cmd[100],response[100];
      sprintf(cmd,"ENBL?\n");
      int rc = comm_driver::query(type,handle,cmd,response); 
      state = atoi(response); 
      return rc;  
   }
   //______________________________________________________________________________
   int get_ntype_output_state(int type,int handle,int &state){
      char cmd[100],response[100];
      sprintf(cmd,"ENBR?\n");
      int rc = comm_driver::query(type,handle,cmd,response); 
      state = atoi(response); 
      return rc;  
   }
   //______________________________________________________________________________
   int get_bnc_amplitude(int type,int handle,double &amp){
      char cmd[100],response[100];
      sprintf(cmd,"AMPL?\n");
      int rc = comm_driver::query(type,handle,cmd,response); 
      amp = atof(response); 
      return rc;  
   }
   //______________________________________________________________________________
   int get_ntype_amplitude(int type,int handle,double &amp){
      char cmd[100],response[100];
      sprintf(cmd,"AMPR?\n");
      int rc = comm_driver::query(type,handle,cmd,response); 
      amp = atof(response); 
      return rc;  
   }
   //______________________________________________________________________________
   int get_frequency(int type,int handle,double &freq){
      char cmd[100],response[100];
      sprintf(cmd,"FREQ?\n");
      int rc = comm_driver::query(type,handle,cmd,response); 
      freq = atof(response); 
      return rc;  
   }
   //______________________________________________________________________________
   int get_error(int type,int handle,char *response){
      const int SIZE = 512; 
      char cmd[SIZE];
      sprintf(cmd,"LERR?\n"); 
      int rc       = comm_driver::query(type,handle,cmd,response); 
      int err_code = atoi(response); 
      rc *= 1; 
      return err_code;  
   }
}
