// plot the data from the digitizers

#include <cstdlib> 
#include <iostream> 
#include <vector>
#include <string> 

#include "TString.h"
#include "TCanvas.h"

#include "gm2fieldImport.h"
#include "gm2fieldGraph.h"

int ConvertToVoltage(int modID,std::vector<double> &x); 
int GetLimits(std::vector<double> x,double &yMin,double &yMax); 

int Plot(){

   int rc              = 0;
   int modID           = 3302;
   int chNum           = 1;   
   const int numTraces = 10;
   
   std::string testDir = "50-Ohm-2"; 

   char path[700],prefix[700];
   sprintf(prefix,"./output/%04d/ch-%02d/%s",modID,chNum,testDir.c_str());
   std::string inpath = "NONE";
 
   std::cout << "Plotting data from " << prefix << std::endl;
 
   std::vector<double> x,y;

   TGraph **g = new TGraph*[numTraces];  

   double yMin[numTraces],yMax[numTraces]; 

   for(int i=0;i<numTraces;i++){
      std::cout << "Processing trace " << i+1 << std::endl;
      sprintf(path,"%s/sis%d_ch-%02d_%02d.csv",prefix,modID,chNum,i+1);
      inpath = path;
      rc = gm2fieldUtil::Import::ImportData2<double>(inpath,"csv",x,y);
      if(rc!=0) return 1;
      rc = GetLimits(y,yMin[i],yMax[i]); 
      // ConvertToVoltage(modID,y); 
      g[i] = gm2fieldUtil::Graph::GetTGraph(x,y);
      gm2fieldUtil::Graph::SetGraphParameters(g[i],20,kBlack);
      g[i]->SetMarkerSize(0.5); 
      // clean up for next trace
      x.clear(); 
      y.clear(); 
   }
  
   TString Title;
   TString xAxisTitle = Form("Time (s)"); 
   TString yAxisTitle = Form("Amplitude (V)"); 

   double xMin = 0; 
   double xMax = 0.05E-3;

   TCanvas *c1 = new TCanvas("c1","SIS Test",1200,600);
   c1->Divide(3,4);

   for(int i=0;i<numTraces;i++){
      Title = Form("Trace %02d",i+1);
      c1->cd(i+1);
      g[i]->Draw("alp"); 
      gm2fieldUtil::Graph::SetGraphLabels(g[i],Title,xAxisTitle,yAxisTitle);
      gm2fieldUtil::Graph::SetGraphLabelSizes(g[i],0.05,0.06);
      g[i]->GetXaxis()->SetLimits(xMin,xMax);  
      // g[i]->GetYaxis()->SetRangeUser(yMin[i],yMax[i]);  
      g[i]->Draw("alp"); 
   } 

   return 0;
}
//_______________________________________________________________________________
int ConvertToVoltage(int modID,std::vector<double> &x){
   const int N = x.size();
   double p[2] = {0,0}; 

   if(modID==3302){
      p[0] = 32599.9;
      p[1] = 30465.9;
   }else if(modID==3316){
      p[0] = 32558.5;
      p[1] = 12629.5;
   }

   for(int i=0;i<N;i++) x[i] = (x[i]-p[0])/p[1]; 
   return 0;
}
//_______________________________________________________________________________
int GetLimits(std::vector<double> x,double &yMin,double &yMax){
   yMin =  500; 
   yMax = -500; 
   const int N = x.size();
   for(int i=0;i<N;i++){
      if(x[i]<yMin) yMin = x[i];
      if(x[i]>yMax) yMax = x[i];
   }

   yMin += 0.5*yMin;
   yMax += 0.5*yMax;
 
   return 0;
}
