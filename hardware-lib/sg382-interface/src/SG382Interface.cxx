#include "SG382Interface.hh"

namespace sg382_interface { 
   //______________________________________________________________________________
   int open_connection(const char *device_path) {
      int rs232_handle=0;
      rs232_handle = open(device_path, O_RDWR | O_NOCTTY | O_NDELAY);
      // rs232_handle = open(device_path, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
      if (rs232_handle < 0) { 
	 printf("[SG382]: ERROR: Failed to open usb->serial port. \n");
	 return -1; 
      }

      if ( tcgetattr(rs232_handle, &old_termios) != 0 ) {
	 printf("[SG382]: ERROR: Failed to read original serial settings.\n");
	 close(rs232_handle);
	 exit(1);
      }

      // 8 data bits, no parity, 1 stop bit, 9600 baud, hdw flow control
      struct termios new_termios;
      // new_termios.c_cflag = CS8 | B9600 | CRTSCTS;
      // new_termios.c_cflag = CS8 | B115200 | CRTSCTS;
      new_termios.c_cflag     &=  ~PARENB;        // Make 8n1  
      new_termios.c_cflag     &=  ~CSTOPB;
      new_termios.c_cflag     &=  ~CSIZE;
      new_termios.c_cflag     |=  CS8;
      new_termios.c_cflag     &=  ~CRTSCTS;       // no flow control 

      // set baud rate 
      cfsetospeed(&new_termios,B115200);
      cfsetispeed(&new_termios,B115200);

      int rc = tcsetattr(rs232_handle, TCSANOW, &new_termios);
      if(rc<0){
	 printf("[SG382]: Something's wrong. ERROR %d \n",rc);
	 return -1;
      }

      tcflush(rs232_handle,TCIOFLUSH);

      usleep(1E5);
      return rs232_handle;
   }
   //______________________________________________________________________________
   int close_connection(int rs232_handle){
      tcsetattr(rs232_handle,TCSANOW,&old_termios);      // restore old settings
      return close(rs232_handle);
   }
   //______________________________________________________________________________
   int write_cmd(int rs232_handle, char *buffer){
      int rc=0;
      int buffer_size = (int)( strlen(buffer) ); 
      rc = write(rs232_handle, buffer, buffer_size); 
      usleep(SG382_SLEEP_TIME);
      write(rs232_handle, "*WAI\n", 5); 
      usleep(SG382_SLEEP_TIME);
      return rc; 
   }
   //______________________________________________________________________________
   int clear_error(int rs232_handle){
      const int SIZE = 100; 
      char buf[SIZE];
      sprintf(buf,"*CLS\n");
      int rc = write_cmd(rs232_handle,buf); 
      return rc; 
   }
   //______________________________________________________________________________
   int get_error(int rs232_handle,char *response){
      const int SIZE = 512; 
      char buf[SIZE];
      sprintf(buf,"LERR?\n"); 
      int rc       = ask(rs232_handle,buf,response); 
      int err_code = atoi(response); 
      rc *= 1; 
      return err_code;  
   }
   //______________________________________________________________________________
   int ask(int rs232_handle,char *in_buffer,char *out_buffer){
      const int SIZE = 512;
      int in_size = (int)( strlen(in_buffer) ); 
      write(rs232_handle,in_buffer,in_size);
      usleep(SG382_SLEEP_TIME);
      int rc = read(rs232_handle,out_buffer,SIZE);
      usleep(SG382_SLEEP_TIME);
      return rc;
   }
   //______________________________________________________________________________
   int set_freq(int rs232_handle,double freq) {
      char freq_str[100];
      sprintf(freq_str, "FREQ %.7lf\n",freq);
      return write_cmd(rs232_handle, freq_str);
   }
   //______________________________________________________________________________
   int set_bnc_amp(int rs232_handle,double amp) {
      char amp_str[100];
      sprintf(amp_str, "AMPL %.7lf\n",amp);
      return write_cmd(rs232_handle, amp_str);
   }
   //______________________________________________________________________________
   int set_ntype_amp(int rs232_handle,double amp) {
      char amp_str[100];
      sprintf(amp_str, "AMPR %.7lf\n",amp);
      return write_cmd(rs232_handle,amp_str);
   }
   //______________________________________________________________________________
   int set_bnc_output_state(int rs232_handle, int flag) {
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
      rc = write_cmd(rs232_handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_ntype_output_state(int rs232_handle, int flag) {
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
      rc = write_cmd(rs232_handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_modulation_state(int rs232_handle, int flag) {
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
      rc = write_cmd(rs232_handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_modulation_function(int rs232_handle, int flag) {
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
      rc = write_cmd(rs232_handle,cmd); 
      return rc;
   }
   //______________________________________________________________________________
   int set_modulation_rate(int rs232_handle,double freq) {
      int rc = 0;
      char freq_str[100];
      sprintf(freq_str, "RATE %.14lf\n", freq);
      rc = write_cmd(rs232_handle,freq_str); 
      return rc;
   }

}
