// test out SIS comms 

#include <cstdlib>
#include <iostream>

#include "sisParameters.h"
#include "SIS3302.hh"
#include "SIS3316.hh"

int PrintToFile(const char *outpath,double clkFreq,std::vector<unsigned short> data); 

int main(){

   // configuration for the digitizer  
   sisParameters_t par; 
   par.moduleBaseAddress = 0x41000000;
   par.clockFrequency    = 10;  
   par.clockFreqUnits    = SISInterface::MHz; 
   par.signalLength      = 60.; 
   par.signalLengthUnits = SISInterface::msec;
   par.channelNumber     = 1; 
   par.clockType         = SISInterface::kExternal; 
   par.multiEventState   = SISInterface::kDisable; 
   par.debug             = false;  

   std::string devPath = "/dev/sis1100_00remote";   // path to the digitizer 

   SIS3316 *my3316 = new SIS3316(par);
   my3316->SetName("SIS3316"); 
   my3316->SetPath( devPath.c_str() );
   my3316->OpenConnection();
   my3316->Initialize();  

   // now lets read some data 
   std::vector<unsigned short> data;

   int rc=0;
   char outpath[512]; 

   const int NEV = 10; 
   for(int i=0;i<NEV;i++){
      my3316->ReadOutData();      // read out the data to its internal buffer 
      my3316->GetData(data);      // pull data out of class 
      sprintf(outpath,"./output/sis%d_%d.csv",my3316->GetModuleID(),i+1);
      rc = PrintToFile(outpath,par.clockFrequency,data); // print to file 
      my3316->ReInitialize();     // prepare for next event 
   } 

   delete my3316;

   return rc;
}
//______________________________________________________________________________
int PrintToFile(const char *outpath,double clkFreq,std::vector<unsigned short> data){
   // print the data to a CSV file 
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
