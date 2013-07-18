#include "Defines.hpp"

// STL includes
#include <fstream>
#include <string>

// Boost includes

// Other 3rd-party includes
#ifdef PARALLEL_MPI
#include "mpi.h"
#endif // end ifdef PARALLEL_MPI

// Includes specific to this code
#include "Driver.hpp"
#include "Parameters.hpp"

namespace Log {

   // TODO : If a write is attempted before the logfile is opened, save the
   //        output in a queue.  Once the file is opened, empty the queue into
   //        the file.  If the queue is non-empty when cleanup() is called,
   //        print the results of the queue to the screen instead.

   // component-scope variables
   const unsigned int log_master = 0;
   std::string log_file;
   std::ofstream lout;

   // =========================================================================
   // Set up

   void setup () {

      // ----------------------------------------------------------------------
      // Initialize the Log component

      // Name of log file
      log_file = Parameters::parameter_with_default<std::string>(
            "Log.log_file", "log.txt");
      log_file = Driver::output_dir + log_file;
#ifdef PARALLEL_MPI
      if (Driver::proc_ID == log_master) {
         lout.open(log_file.c_str());
      }
#else // PARALLEL_MPI
      lout.open(log_file.c_str());
#endif // PARALLEL_MPI

   }

   // =========================================================================
   // Clean up

   void cleanup () {

      // Close the log file
#ifdef PARALLEL_MPI
      if (Driver::proc_ID == log_master) {
         lout << std::endl << std::string(79, '_') << std::endl;
         lout << "Program complete" << std::endl;
         lout.close();
      }
#else // PARALLEL_MPI
      lout << std::endl << std::string(79, '_') << std::endl;
      lout << "Program complete" << std::endl;
      lout.close();
#endif // PARALLEL_MPI

   }

   // =========================================================================
   // Write to the log file

   void write_single(std::string message) {
#ifdef PARALLEL_MPI
      if (Driver::proc_ID == log_master) {
         lout << message;
      }
#else // PARALLEL_MPI
      lout << message;
#endif // PARALLEL_MPI
   }

   void write_all(std::string message) {
#ifdef PARALLEL_MPI
      // Declare variables
      int length, max_length;
      char *send_buf, *recv_buf;

      // MPI_Allreduce to determine length of longest message
      length = message.length();
      MPI_Allreduce(&length, &max_length, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

      // Pad message to max_length
      message.resize(max_length, '\0');

      // Allocate a character array of max_length
      send_buf = (char*) message.c_str();
      recv_buf = new char[Driver::n_procs*max_length];

      // MPI_Gather the messages into a read buffer
      MPI_Gather(send_buf, max_length, MPI_CHAR,
                 recv_buf, max_length, MPI_CHAR, log_master, MPI_COMM_WORLD);

      // If you are the master:
      if (Driver::proc_ID == log_master) {
         // Parse the read buffer into individual messages
         for (int i = 0; i < Driver::n_procs; i++) {
            for (int j = 0; j < max_length; j++) {
               if (recv_buf[i*max_length+j] == '\0') {
                  break;
               }
               lout << recv_buf[i*max_length+j];
            }
         }
      }
#else // PARALLEL_MPI
      lout << message;
#endif // PARALLEL_MPI
   }

   // =========================================================================
   // Flush

   void flush() {
      lout.flush();
   }

}
