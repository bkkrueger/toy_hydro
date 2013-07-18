#include "Defines.hpp"

// STL includes
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

// Boost includes
#include <boost/filesystem.hpp>

// Other 3rd-party includes
#ifdef PARALLEL_MPI
#include "mpi.h"
#endif // end ifdef PARALLEL_MPI

// Includes specific to this code
#include "Driver.hpp"
#include "Grid.hpp"
#include "Hydro.hpp"
#include "InitConds.hpp"
#include "Log.hpp"
#include "Monitor.hpp"
#include "Parameters.hpp"
#include "Support.hpp"

namespace fs = boost::filesystem;

namespace Driver {

   // component-scope variables
   unsigned int n_step = 0;
   DelayedConst<unsigned int> max_steps;
   DelayedConst<unsigned int> n_width;

   double dt;
   double time = 0.0;
   DelayedConst<double> tmax;

   double output_dt = 0.0;

   std::string output_dir;
   std::string restart_dir;

#ifdef PARALLEL_MPI
   DelayedConst<int> n_procs, proc_ID;
   DelayedConst<unsigned int> p_width;
   DelayedConst<int> neigh_lo, neigh_hi;
#endif // end ifdef PARALLEL_MPI

   // =========================================================================
   // Set up

   void setup (int argc, char *argv[]) {

      // ----------------------------------------------------------------------
      // Declare variables
      std::string param_file_name;
      bool changed_pf_name;
#ifdef PARALLEL_MPI
      int mpi_return;
#endif // end ifdef PARALLEL_MPI
      int temp_int;

      // ----------------------------------------------------------------------
      // MPI

#ifdef PARALLEL_MPI
      mpi_return = MPI_Init(&argc, &argv);
      if (mpi_return != MPI_SUCCESS) {
         std::cerr << "MPI_Init failed" << std::endl;
         MPI_Abort(MPI_COMM_WORLD, mpi_return);
      }
      MPI_Comm_size(MPI_COMM_WORLD, &temp_int);
      n_procs = temp_int;
      MPI_Comm_rank(MPI_COMM_WORLD, &temp_int);
      proc_ID = temp_int;
      p_width = floor(log10(n_procs)) + 1;
      if (proc_ID == 0) {
         neigh_lo = n_procs - 1;
      } else {
         neigh_lo = proc_ID - 1;
      }
      if (proc_ID == n_procs - 1) {
         neigh_hi = 0;
      } else {
         neigh_hi = proc_ID + 1;
      }
#endif // end ifdef PARALLEL_MPI

      // ----------------------------------------------------------------------
      // Initialize the Parameter component

      Parameters::setup(argc, argv);

      // ----------------------------------------------------------------------
      // Initialize the Driver component

      // Maximum number of time steps
      max_steps = Parameters::parameter_with_default<unsigned int>(
            "Driver.max_steps", 1000000);
      n_width = floor(log10(max_steps)) + 1;

      // Output directory
      output_dir = Parameters::parameter_with_default<std::string>(
            "Driver.output_dir", "output/");
      // Modify the output directory name as needed
      if (!output_dir.empty()) {
         if (*output_dir.rbegin() != '/') {
            output_dir.append("/");
         }
      } else {
         output_dir = "./";
      }
      // Ensure the output directory exists
      if (!fs::exists(output_dir)) {
         fs::create_directories(output_dir);
      }

      // Time between saving output files
      output_dt = Parameters::parameter_with_default<double>(
            "Driver.output_dt", 0.0);

      // Restart from which directory
      restart_dir = Parameters::parameter_with_default<std::string>(
            "Driver.restart_dir", "");

      // Current time
      time = 0.0;

      // Final time
      tmax = Parameters::parameter_without_default<double>("Driver.tmax");

      // ----------------------------------------------------------------------
      // Initialize other components

      // Initialize
      Log::setup();
      Grid::setup();
      Hydro::setup();
      InitConds::setup();
      Monitor::setup();

      std::stringstream ss;
      ss.clear();
      ss.str("");
      ss << "Processor " << proc_ID << " : neighbors = < ";
      ss << neigh_lo << " | " << neigh_hi << " >" << std::endl;
      Log::write_all(ss.str());

      // Print the used parameters
      Parameters::print_used_parameters();

      // Print the unused parameters
      Parameters::print_unused_parameters();

   }

   // =========================================================================
   // Clean up

   void cleanup () {

      // Have all the other sections run their clean-up routines
      // --> Reverse order from setup in case of dependencies
      Monitor::cleanup();
      InitConds::cleanup();
      Hydro::cleanup();
      Grid::cleanup();
      Parameters::cleanup();
      Log::cleanup();

#ifdef PARALLEL_MPI
      MPI_Finalize();
#endif // end ifdef PARALLEL_MPI
   }

   // =========================================================================
   // Compute the time step
   // --> Each component may have restrictions on the time step; this collects
   //     all the time step restrictions and determines the maximum allowed
   //     time step consistent with all those restrictions.

   double compute_time_step() {
      double dt;
      double hydro_dt = Hydro::compute_time_step();
      dt = hydro_dt;
      /* For other components:
      double ???_dt = ???::compute_time_step();
      if (???_dt < dt) {
         dt = ???_dt;
      }
      */
      return dt;
   }

   // =========================================================================
   // The main evolution loop

   void main (int argc, char *argv[]) {

      // ----------------------------------------------------------------------
      // Declare variables

      double prev_write = -1;
      double curr_write;
      bool do_write;
      std::string outname;
      const unsigned int w = 13;
      std::stringstream ss;

      // ----------------------------------------------------------------------
      // Initialize

      setup(argc, argv);

      // ----------------------------------------------------------------------
      // Evolution loop

      Log::write_single(std::string(79,'_')+"\nEvolution loop:\n\n");

      for (; n_step < max_steps; n_step++) {

         // Exceeded maximum time
         if (time >= tmax) {
            break;
         }

         // Boundary condition fill
         Grid::fill_boundary_conditions();

         // Unexpected exit (e.g., like FLASH's "touch .dump_restart")
         // TODO

         // Write output
         do_write = false;
         // Condition based on time (print every output_dt seconds)
         if ((time == 0) || (output_dt > 0)) {
            curr_write = floor(time / output_dt);
            if (curr_write > prev_write) {
               do_write = true;
            }
            prev_write = curr_write;
         }
         // TODO - Output based on steps
         // TODO - Output based on wall clock
         // Do the actual write
         if (do_write) {
            outname = Grid::write_data();
            ss.clear();
            ss.str("");
            ss << "OUTPUT : wrote output \"" << outname << "\"" << std::endl;
            Log::write_single(ss.str());
         }

         // Monitor write
         Monitor::write_to_monitor();

         // Compute step size
         dt = compute_time_step();

         // Print logfile note
         ss.clear();
         ss.str("");
         ss << "n = " << std::setw(n_width) << std::right << n_step;
         ss << "; t = " << std::setw(w) << std::scientific << time;
         ss << "; dt = " << std::setw(w) << std::scientific << dt;
         ss << std::endl;
         Log::write_single(ss.str());

         // Evolve a single step
         Hydro::one_step();

         // Update time
         time = time + dt;

      }

      // Final write
      Grid::fill_boundary_conditions();   // TODO --- only if printing guard
      ss.clear();
      ss.str("");
      ss << "n = " << std::setw(n_width) << std::right << n_step;
      ss << "; t = " << std::setw(w) << std::scientific << time;
      ss << std::endl;
      Log::write_single(ss.str());
      outname = Grid::write_data();
      ss.clear();
      ss.str("");
      ss << "OUTPUT : wrote output \"" << outname << "\"" << std::endl;
      Log::write_single(ss.str());
      Monitor::write_to_monitor();

      // ----------------------------------------------------------------------
      // Finalize

      cleanup();

   }

}
