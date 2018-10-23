// test out SIS comms 

#include <cstdlib>
#include <iostream>

#include "sisParameters.h"
#include "SIS3302.hh"

int main(){

   sisParameters_t myPar; 

   SIS3302 *my3302 = new SIS3302(); 

   std::cout << "Nothing yet" << std::endl;

   delete my3302;

   return 0;
}
