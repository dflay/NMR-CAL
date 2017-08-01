// test the SG382 comms  

#include <cstdlib> 
#include <iostream>
#include <fstream>
#include <string>

#include "SG382.hh"

int main(){

   int rc=-1;
   char msg[512];  

   SG382 *mySG382 = new SG382();
   mySG382->SetProtocol(CommDriver::kRS232); 
   mySG382->SetName("SRS SG382");
   mySG382->SetPath("/dev/ttyUSB1");
 
   rc = mySG382->OpenConnection();
   if(rc==0){
      std::cout << "Connection opened!" << std::endl;
      rc = mySG382->GetDeviceID(msg);
      std::cout << msg << std::endl;
   }else{
      std::cout << "Cannot open connection!" << std::endl;
      return 1;
   }

   strcpy(msg,""); 
   rc = mySG382->CheckErrors(msg); 
   std::cout << msg << std::endl;

   rc = mySG382->SetFrequency("61.79 MHz");
   rc = mySG382->SetNTypeAmplitude("0.25 Vpp");  

   double myFreq=0;
   rc = mySG382->GetFrequency(myFreq); 
   std::cout << "Freq = " << myFreq << std::endl;

   double myAmpl=0;
   rc = mySG382->GetNTypeAmplitude(myAmpl); 
   std::cout << "Ampl = " << myAmpl << std::endl;

   rc = mySG382->SetNTypeAmplitude("0 Vpp");

   rc = mySG382->ClearErrors();  
   rc = mySG382->CloseConnection(); 

   delete mySG382;  

   return 0;
}
