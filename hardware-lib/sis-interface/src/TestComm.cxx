// test out SIS comms 

#include <cstdlib>
#include <iostream>

#include "sisParameters.h"
#include "SIS3302.hh"
#include "SIS3316.hh"

int PrintToFile(const char *outpath,sisParameters_t par,std::vector<double> data); 

int main(){

   int rc=0;
 
   // configuration for the digitizer  
   sisParameters_t par; 
   // par.moduleBaseAddress = 0x41000000;  // for the 3316
   par.moduleBaseAddress = 0x30000000;   // for the 3302 
   par.clockFrequency    = 10;  
   par.clockFreqUnits    = SISInterface::MHz; 
   par.signalLength      = 60.; 
   par.signalLengthUnits = SISInterface::msec;
   par.outputUnits       = SISInterface::Volts; 
   par.channelNumber     = 1;
   par.numberOfEvents    = 10; 
   par.clockType         = SISInterface::kExternal; 
   par.multiEventState   = SISInterface::kDisable; 
   par.debug             = true;  

   std::string devPath = "/dev/sis1100_00remote";   // path to the digitizer 

   // SIS3316 *myADC = new SIS3316(par);
   SIS3302 *myADC = new SIS3302(par);
   myADC->SetPath( devPath.c_str() );
   myADC->OpenConnection();
   myADC->Initialize();

   // now lets read some data 
   std::vector<double> data;

   char outpath[512]; 

   const int NEV = par.numberOfEvents; 
   for(int i=0;i<NEV;i++){
      std::cout << "Processing event " << i+1 << std::endl;
      rc = myADC->ReadOutData(data);
      if(rc!=0) break; 
      sprintf(outpath,"./output/sis%d_ch-%02d_%02d.csv",myADC->GetModuleID(),par.channelNumber,i+1);
      rc = PrintToFile(outpath,par,data); // print to file 
      myADC->ReInitialize();      
      data.clear();
   } 

   myADC->CloseConnection(); 
   delete myADC;
  
   std::cout << "Test complete!" << std::endl;
 
   return rc;
}
//______________________________________________________________________________
int PrintToFile(const char *outpath,sisParameters_t par,std::vector<double> data){
   // print the data to a CSV file 
   double sf=1;
   if( par.clockFreqUnits==SISInterface::Hz  ) sf = 1;
   if( par.clockFreqUnits==SISInterface::kHz ) sf = 1E+3;
   if( par.clockFreqUnits==SISInterface::MHz ) sf = 1E+6;
   if( par.clockFreqUnits==SISInterface::GHz ) sf = 1E+9;
   double clkFreq = par.clockFrequency*sf; 

   double time=0;
   const int N = data.size();
   char outStr[200];
   std::ofstream outfile;
   outfile.open(outpath);
   if( outfile.fail() ){
      std::cout << "Cannot open the file: " << outpath << std::endl;
      return 1;
   }else{
      for(int i=0;i<N;i++){
         time = ( (double)i )/clkFreq;
         sprintf(outStr,"%.7lf,%.7lf",time,(double)data[i]); 
         outfile << outStr << std::endl;
      }
      outfile.close();
   }
   return 0; 
}
