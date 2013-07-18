#include "Defines.hpp"

#include <exception>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include "Driver.hpp"

namespace pt = boost::property_tree;

int main(int argc, char *argv[]) {
   try {
      Driver::main(argc, argv);
   } catch (pt::ptree_error &e) {
      std::cerr << e.what() << std::endl;
      return 1;
   } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return 1;
   } catch (...) {
      std::cerr << "ERROR: Caught an unknown exception in Main::main.";
      std::cerr << std::endl;
      return 1;
   }
   return 0;
}
