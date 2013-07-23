#include "Defines.hpp"

#ifdef PARALLEL_MPI
#include "mpi.h"
#endif // end ifdef PARALLEL_MPI

#include <exception>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include "Driver.hpp"

namespace pt = boost::property_tree;

int main(int argc, char *argv[]) {
   try {
      Driver::evolution_loop(argc, argv);
   } catch (pt::ptree_error &e) {
      std::cerr << e.what() << std::endl;
#ifdef PARALLEL_MPI
      MPI_Finalize();
#endif // end ifdef PARALLEL_MPI
      return 1;
   } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
#ifdef PARALLEL_MPI
      MPI_Finalize();
#endif // end ifdef PARALLEL_MPI
      return 1;
   } catch (...) {
      std::cerr << "ERROR: Caught an unknown exception in Main::main.";
      std::cerr << std::endl;
#ifdef PARALLEL_MPI
      MPI_Finalize();
#endif // end ifdef PARALLEL_MPI
      return 1;
   }
   return 0;
}
