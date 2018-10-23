// test out SIS comms 

#include <cstdlib>
#include <iostream>

#include "sisParameters.h"
#include "SIS3302.hh"
#include "SIS3316.hh"

int main(){

   sisParameters_t myPar; 

   SIS3302 *my3302 = new SIS3302(); 
   SIS3316 *my3316 = new SIS3316(); 

   std::cout << "Nothing yet" << std::endl;

   delete my3302;
   delete my3316;

   return 0;
}
