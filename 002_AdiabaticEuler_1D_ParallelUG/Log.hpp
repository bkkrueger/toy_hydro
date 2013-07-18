#include "Defines.hpp"

// STL includes
#include <sstream>
#include <string>

// Boost includes

// Other 3rd-party includes

// Includes specific to this code

namespace Log {

   extern const unsigned int log_master;
   extern bool initialized;
   extern std::stringstream buffer;
   extern std::ofstream lout;

   // =========================================================================
   // Set up

   void setup ();

   // =========================================================================
   // Clean up

   void cleanup ();

   // =========================================================================
   // Write to the log file

   template <typename T>
   void write_single(T message) {
#ifdef PARALLEL_MPI
      if (Driver::proc_ID == log_master) {
#endif // PARALLEL_MPI
         if (initialized) {
            lout << message;
         } else {
            buffer << message;
         }
#ifdef PARALLEL_MPI
      }
#endif // PARALLEL_MPI
   }


   void write_all(std::string message);

   void flush();

}
