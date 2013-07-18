#include <iostream>
#include "Support.hpp"

int main (int argc, char *argv[]) {
   DelayedConst<double> d1;
   DelayedConst<int> i1;
   int i2;
   double d2;
   i1 = 5;
   d1 = 1.7;
   i2 = i1;
   d2 = d1;
   //i1 = 18;    // should give an assertion error
   i2 = d1;
   std::cout << i1 << " " << i2 << " " << d1 << " " << d2 << std::endl;
   return 0;
}
