#include <iostream>
#include "Array.hpp"

int main (int argc, char *argv[]) {

   Array x;
   Array y(5);
   Array z(-12,3);

   std::cout << "declarations:" << std::endl;
   std::cout << "  x: \"Array x;\"" << std::endl;
   std::cout << "  y: \"Array y(5);\"" << std::endl;
   std::cout << "  z: \"Array z(-12,3);\"" << std::endl;

   std::cout << "size:" << std::endl;
   std::cout << "  x: " << x.size() << std::endl;
   std::cout << "  y: " << y.size() << std::endl;
   std::cout << "  z: " << z.size() << std::endl;

   std::cout << "limits:" << std::endl;
   std::cout << "  x: (" << x.lower_limit();
   std::cout << "," << x.upper_limit() << ")" << std::endl;
   std::cout << "  y: (" << y.lower_limit();
   std::cout << "," << y.upper_limit() << ")" << std::endl;
   std::cout << "  z: (" << z.lower_limit();
   std::cout << "," << z.upper_limit() << ")" << std::endl;

   for (int i = x.lower_limit(); i < x.upper_limit(); i++) {
      x[i] = i;
   }
   for (int i = y.lower_limit(); i < y.upper_limit(); i++) {
      y[i] = i;
   }
   for (int i = z.lower_limit(); i < z.upper_limit(); i++) {
      z[i] = i;
   }

   std::cout << "arrays:" << std::endl;
   std::cout << "  x: [";
   for (int i = x.lower_limit(); i < x.upper_limit(); i++) {
      std::cout << " " << x[i];
   }
   std::cout << " ]" << std::endl;
   std::cout << "  y: [";
   for (int i = y.lower_limit(); i < y.upper_limit(); i++) {
      std::cout << " " << y[i];
   }
   std::cout << " ]" << std::endl;
   std::cout << "  z: [";
   for (int i = z.lower_limit(); i < z.upper_limit(); i++) {
      std::cout << " " << z[i];
   }
   std::cout << " ]" << std::endl;

   x.resize(82,85);
   std::cout << "resized: \"x.resize(82,85);\"" << std::endl;
   std::cout << "   size: " << x.size() << std::endl;
   std::cout << "   limits: (" << x.lower_limit() << ",";
   std::cout << x.upper_limit() << ")" << std::endl;
   
   x.resize(0);
   std::cout << "resized: \"x.resize(0);\"" << std::endl;
   std::cout << "   size: " << x.size() << std::endl;
   std::cout << "   limits: (" << x.lower_limit() << ",";
   std::cout << x.upper_limit() << ")" << std::endl;

   return 0;
}
