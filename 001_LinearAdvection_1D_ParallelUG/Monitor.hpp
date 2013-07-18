#ifndef MONITOR_HPP
#define MONITOR_HPP

#include "Defines.hpp"

// STL includes
#include <fstream>

// Boost includes

// Includes specific to this code

//#define MONITOR_CONVERGENCE  // Uncomment to monitor L1, L2, Linf errors

namespace Monitor {

   // component-scope variables
   extern std::ofstream mout;

   // =========================================================================
   // Set up

   void setup ();

   // =========================================================================
   // Compute quantities to be monitored

   void write_to_monitor();

   // =========================================================================
   // Clean up

   void cleanup();

}

#endif
