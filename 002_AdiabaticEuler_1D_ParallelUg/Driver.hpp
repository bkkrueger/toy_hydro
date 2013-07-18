#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "Defines.hpp"

// STL includes
#include <cmath>
#include <fstream>
#include <string>

// Boost includes

// Includes specific to this code
#include "Support.hpp"

namespace Driver {

   // component-scope variables
   extern unsigned int n_step;
   extern DelayedConst<unsigned int> max_steps;
   extern DelayedConst<unsigned int> n_width;

   extern double dt;
   extern double time;
   extern DelayedConst<double> tmax;

   extern double output_dt;

   extern std::string output_dir;
   extern std::string restart_dir;

#ifdef PARALLEL_MPI
   extern DelayedConst<int> n_procs, proc_ID;
   extern DelayedConst<unsigned int> p_width;
#endif // end ifdef PARALLEL_MPI

   // =========================================================================
   // Set up

   void setup (int argc, char *argv[]);

   // =========================================================================
   // Clean up

   void cleanup ();

   // =========================================================================
   // Compute the time step

   double compute_time_step();

   // =========================================================================
   // The main evolution loop

   void evolution_loop (int argc, char *argv[]);

}

#endif
