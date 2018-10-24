// test out SIS comms 

#include <cstdlib>
#include <iostream>

#include "sisParameters.h"
#include "SIS3302.hh"
#include "SIS3316.hh"

int PrintToFile(const char *outpath,sisParameters_t par,std::vector<unsigned short> data); 

int main(){

   int rc=0;
 
   // configuration for the digitizer  
   sisParameters_t par; 
   par.moduleBaseAddress = 0x41000000;
   par.clockFrequency    = 10;  
   par.clockFreqUnits    = SISInterface::MHz; 
   par.signalLength      = 60.; 
   par.signalLengthUnits = SISInterface::msec;
   par.channelNumber     = 1;
   par.numberOfEvents    = 1; 
   par.clockType         = SISInterface::kExternal; 
   par.multiEventState   = SISInterface::kDisable; 
   par.debug             = true;  

   std::string devPath = "/dev/sis1100_00remote";   // path to the digitizer 

   SIS3316 *my3316 = new SIS3316(par);
   my3316->SetPath( devPath.c_str() );
   my3316->OpenConnection();
   my3316->Initialize(); 

   // now lets read some data 
   std::vector<unsigned short> data;

   char outpath[512]; 

   const int NEV = 10; 
   for(int i=0;i<NEV;i++){
      rc = my3316->ReadOutData(data);       
      // if(rc!=0) break; 
      sprintf(outpath,"./output/sis%d_%02d.csv",my3316->GetModuleID(),i+1);
      rc = PrintToFile(outpath,par,data); // print to file 
      // prepare for next event
      my3316->ReInitialize();      
      data.clear();
   } 

   my3316->CloseConnection(); 

   delete my3316;

   return rc;
}
//______________________________________________________________________________
int PrintToFile(const char *outpath,sisParameters_t par,std::vector<unsigned short> data){
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
