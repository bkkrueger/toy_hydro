// TODO --- need to modify to handle parallel; see Log for comparison
#include "Defines.hpp"

// STL includes
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>

// Boost includes

// Includes specific to this code
#include "Driver.hpp"
#include "Grid.hpp"
#include "Hydro.hpp"
#include "Monitor.hpp"
#include "Parameters.hpp"

namespace Monitor {

   // component-scope variables
   std::string monitor_file;
   std::ofstream mout;

   const unsigned int w = 13;

   // =========================================================================
   // Set up

   void setup () {

      // ----------------------------------------------------------------------
      // Declare variables

      // ----------------------------------------------------------------------
      // Get parameters

      monitor_file = Parameters::parameter_with_default<std::string>(
            "Monitor.monitor_file", "monitor.dat");
      monitor_file = Driver::output_dir + monitor_file;
      mout.open(monitor_file.c_str(), std::fstream::out | std::fstream::app);

      // ----------------------------------------------------------------------
      // Write the header for the monitor file

      mout << std::endl << "#" << std::string(78, '=') << std::endl;
      mout << "# ";
      mout << std::setw(w) << "time";
      mout << std::endl;

   }

   // =========================================================================
   // Compute quantities to be monitored

   void write_to_monitor() {

      // ----------------------------------------------------------------------
      // Compute quantities to be monitored

      mout << "  " << std::setw(w) << std::scientific << Driver::time;
      mout << std::endl;

   }

   // =========================================================================
   // Clean up

   void cleanup() {
      mout.close();
   }

}
