/*****************************************************************************\
 * Driver.cpp                                                                *
 *                                                                           *
 * This file contains the driver for the code.  The driver is responsible    *
 * for running the core loop for the code, as well as managing the setup and *
 * cleanup of the whole code.  It also holds quantities important to the     *
 * whole code, such as the current time or (if running in parallel) the      *
 * processor ID.                                                             *
\*****************************************************************************/

// Always include Defines.hpp first, as other includes may depend on
// definitions stored there.
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
#include "Parameters.hpp"
#include "Support.hpp"

namespace Driver {

   // =========================================================================
   // component-scope variables

   // The current step number
   unsigned int n_step = 0;

   // The maximum number of steps that the core loop is allowed to take
   DelayedConst<unsigned int> max_steps;

   // A formatting constant: number of digits necessary to write a step number
   DelayedConst<unsigned int> n_width;

   // The current time step (stepping from time to time+dt)
   double dt;

   // The current time
   double time = 0.0;

   // The maximum simulation time
   DelayedConst<double> tmax;

   // The output frequency
   // - print every output_dt seconds
   double output_dt = 0.0;
   // - print every output_dn steps
   double output_dn = 0;

   // The directory to write the output files
   std::string output_dir;

   // The directory containing the files for a restart
   std::string restart_dir;

#ifdef PARALLEL_MPI
   // The number of processors and the ID of the local processor
   DelayedConst<int> n_procs, proc_ID;

   // A formatting constant: number of digits necessary to write a processor ID
   DelayedConst<unsigned int> p_width;
#endif // end ifdef PARALLEL_MPI

   // =========================================================================
   // Set up
   //    The set up routine is responsible for managing the start up of the
   // overall program.  It handles any MPI set up, sets up the Driver component
   // itself, and calls the set up routines for the other components.  Set up
   // for a component includes getting any runtime parameters from the
   // configuration file and any other steps necessary to prepare the component
   // for use.
   //
   // Arguments:
   // - argc: a copy of argc from main
   // - argv: a copy of argc from main
   //
   // Returns:
   // - none
   //
   // Side effects:
   // - MPI (if used) is initialized
   // - All other components have their set up routines called
   // - Parameters in the Driver component are loaded from the config file

   void setup (int argc, char *argv[]) {

      // ----------------------------------------------------------------------
      // Declare variables
#ifdef PARALLEL_MPI
      int mpi_return;
      int temp_int;
#endif // end ifdef PARALLEL_MPI

      // ----------------------------------------------------------------------
      // MPI

#ifdef PARALLEL_MPI
      // Initialize
      mpi_return = MPI_Init(&argc, &argv);
      if (mpi_return != MPI_SUCCESS) {
         std::cerr << "MPI_Init failed" << std::endl;
         MPI_Abort(MPI_COMM_WORLD, mpi_return);
      }
      // Get the number of processors and the local processor ID
      MPI_Comm_size(MPI_COMM_WORLD, &temp_int);
      n_procs = temp_int;
      MPI_Comm_rank(MPI_COMM_WORLD, &temp_int);
      proc_ID = temp_int;
      // Useful formatting constant
      p_width = fmax(floor(log10(n_procs)) + 1, 6);
#endif // end ifdef PARALLEL_MPI

      // ----------------------------------------------------------------------
      // Initialize the Parameter component
      // --> This gets special treatment as all other components (including the
      //     Driver itself) needs to read parameter values during setup, so the
      //     Parameter component must be set up before any others.

      Parameters::setup(argc, argv);

      // ----------------------------------------------------------------------
      // Initialize the Driver component

      // Maximum number of time steps
      max_steps = Parameters::get_optional<unsigned int>(
            "Driver.max_steps", 999999);
      // Useful formatting constant
      n_width = fmax(floor(log10(max_steps)) + 1, 6);

      // Output directory
      output_dir = Parameters::get_optional<std::string>(
            "Driver.output_dir", "output/");
      // Modify the output directory name as needed
      if (output_dir.empty()) {
         // Empty directory name --> write to current directory
         output_dir = "./";
      } else {
         // Ensure the directory name ends with a slash
         if (*output_dir.rbegin() != '/') {
            output_dir.append("/");
         }
      }
      // Ensure the output directory exists
      if (!boost::filesystem::exists(output_dir)) {
#ifdef PARALLEL_MPI
         if (proc_ID == 0) {
#endif
            boost::filesystem::create_directories(output_dir);
#ifdef PARALLEL_MPI
         }
#endif
      }

      // Time between saving output files
      output_dt = Parameters::get_optional<double>("Driver.output_dt", 0.0);
      output_dn = Parameters::get_optional<unsigned int>("Driver.output_dn",0);

      // Restart from which directory
      restart_dir = Parameters::get_optional<std::string>(
            "Driver.restart_dir", "");
      // Modify the restart directory name as needed
      if (!restart_dir.empty()) {
         // Ensure the directory name ends with a slash
         if (*restart_dir.rbegin() != '/') {
            restart_dir.append("/");
         }
      }

      // Current time
      time = 0.0;

      // Final time
      tmax = Parameters::get_required<double>("Driver.tmax");

      // ----------------------------------------------------------------------
      // Initialize other components

      // Initialize
      Log::setup();
Log::flush();
      Grid::setup();
Log::flush();
      Hydro::setup();
Log::flush();
      InitConds::setup();
Log::flush();

      // Print the used parameters
      Parameters::print_used_parameters();

      // Print the unused parameters
      Parameters::print_unused_parameters();

   }

   // =========================================================================
   // Clean up
   //    The clean up routine is responsible for managing the shut down of the
   // overall program.  It closes down all other components, itself, and MPI
   // (if necessary).
   //
   // Arguments:
   // - none
   //
   // Returns:
   // - none
   //
   // Side effects:
   // - MPI (if used) is finalized
   // - All other components have their clean up routines called

   void cleanup () {

      // Have all the other sections run their clean-up routines
      // --> Reverse order from setup in case of dependencies
      InitConds::cleanup();
      Hydro::cleanup();
      Grid::cleanup();
      Parameters::cleanup();
      Log::cleanup();   // Special case -- this seals off the Log file, so it
                        //                 needs to finalize last even though
                        //                 it did not initialize first

#ifdef PARALLEL_MPI
      MPI_Finalize();
#endif // end ifdef PARALLEL_MPI
   }

   // =========================================================================
   // Compute the time step
   //    This routine packages up the time step restriction calculations into a
   // single call (keeps the main loop clean).  Each component may have its own
   // restriction on the time step, so this computes a time step consistent
   // with all such restrictions.
   //
   // Arguments:
   // - none
   //
   // Returns:
   // - the maximum allowed time step
   //
   // Side effects:
   // - none

   double compute_time_step() {
      double dt;
      double hydro_dt = Hydro::compute_time_step();

      dt = hydro_dt;

      /* For other components:
      double ???_dt = ???::compute_time_step();
      if (???_dt < dt) {
         dt = ???_dt;
      }*/

      return dt;
   }

   // =========================================================================
   // The main evolution loop
   //    This function runs the main evolution loop and performs any important
   // housekeeping surrounding or during the loop.
   //
   // Arguments:
   // - argc: from the main function of the program
   // - argv: from the main function of the program
   //
   // Returns:
   // - none
   //
   // Side effects:
   // - This effectively runs the entire program, so the side effects are many.

   void evolution_loop (int argc, char *argv[]) {

      // ----------------------------------------------------------------------
      // Declare variables

      int prev_write_dt, curr_write_dt;
      int prev_write_dn, curr_write_dn;
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

      prev_write_dt = -1;
      prev_write_dn = -1;

      for (; n_step < max_steps; n_step++) {

         // Exceeded maximum time
         if (time >= tmax) {
            break;
         }

         // Boundary condition fill
         Grid::fill_boundary_conditions();

         // Controlled exit on an alternate condition (not time or steps, but
         // by an external action by the user)
         if (boost::filesystem::exists("_force_quit")) {
            Log::write_single("--- FORCED EXIT ---\n");
            break;
         }

         // Write output
         do_write = false;
         // Write the first step
         if (time == 0) {
            do_write = true;
         }
         // Condition based on time (print every output_dt seconds)
         if ((!do_write) && (output_dt > 0.0)) {
            curr_write_dt = int(floor(time / output_dt));
            if (curr_write_dt > prev_write_dt) {
               do_write = true;
            }
            prev_write_dt = curr_write_dt;
         }
         // Condition based on number of steps
         if ((!do_write) && (output_dn > 0)) {
            curr_write_dn = n_step / output_dn;
            if (curr_write_dn > prev_write_dn) {
               do_write = true;
            }
            prev_write_dn = curr_write_dn;
         }
         // Do the actual write
         if (do_write) {
            outname = Grid::write_data();
            ss.clear();
            ss.str("");
            ss << "OUTPUT : wrote output \"" << outname << "\"" << std::endl;
            Log::write_single(ss.str());
         }

         // Compute step size
         dt = compute_time_step();

         // Print logfile note marking the time step
         ss.clear();
         ss.str("");
         ss << "n = " << std::setw(n_width) << std::right << n_step;
         ss << "; t = " << std::setw(w) << std::scientific << time;
         ss << "; dt = " << std::setw(w) << std::scientific << dt;
         ss << std::endl;
         Log::write_single(ss.str());

         // Evolve a single step of hydrodynamics
         Hydro::one_step();

         // Update time
         time = time + dt;

      }

      // Final write
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

      // ----------------------------------------------------------------------
      // Finalize

      cleanup();

   }

}
