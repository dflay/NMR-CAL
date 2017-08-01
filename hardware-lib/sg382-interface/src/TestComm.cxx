// test the SG382 comms  

#include <cstdlib> 
#include <iostream>
#include <fstream>
#include <string>

#include "SG382.hh"

int main(){

   int rc=-1; 
   char freq[100],ampl[100];

   sprintf(freq,"%s","61.79 MHz");
   sprintf(ampl,"%s","0.5 Vpp");

   SG382 *mySG382 = new SG382();
   mySG382->SetProtocol(CommDriver::kRS232); 
   mySG382->SetName("SRS SG382");
   mySG382->SetPath("/dev/ttyUSB1"); 
   rc = mySG382->OpenConnection();
   rc = mySG382->SetFrequency(freq);
   rc = mySG382->SetNTypeAmp(ampl);  
   rc = mySG382->CloseConnection(); 

   double myFreq=0;
   rc = mySG382->GetFrequency(myFreq); 
   std::cout << "Freq = " << myFreq << std::endl;

   delete mySG382;  

   return 0;
}
