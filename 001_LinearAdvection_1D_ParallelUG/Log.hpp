#include "Defines.hpp"

// STL includes
#include <string>

// Boost includes

// Other 3rd-party includes

// Includes specific to this code

namespace Log {

   // =========================================================================
   // Set up

   void setup ();

   // =========================================================================
   // Clean up

   void cleanup ();

   // =========================================================================
   // Write to the log file

   void write_single(std::string message);

   void write_all(std::string message);

   void flush();

}
