// TODO : Do I want to turn this into a class to allow public/private?  There
// are also unnamed namespaces, but I don't know the usage of those so I'm not
// yet confident enough to use them myself.

// STL includes
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

// Includes specific to this code
#include "Driver.hpp"
#include "Grid.hpp"
#include "Hydro.hpp"
#include "InitConds.hpp"
#include "Monitor.hpp"

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

namespace Driver {

   // parameter listings (all are case-insensitive PropertyTrees from Boost)
   pt::iptree param_used;     // parameters used by the code
   pt::iptree param_unused;   // parameters not used by the code
   pt::iptree param_changed;  // have parameters changed from default?

   // component-scope variables
   unsigned int n_step = 0;
   unsigned int max_steps = 0;
   unsigned int n_width;

   double dt;
   double time = 0.0;
   double tmax = 0.0;

   double output_dt = 0.0;

   std::string output_dir;
   std::string restart_dir;
   std::string log_file;
   std::ofstream lout;

   // =========================================================================
   // Parameter printing

   // Print the parameters from a section that has been processed
   // - Inputs:
   //   - name of the section to print
   // - Outputs:
   //   - none
   // - Side Effects:
   //   - none
   void print_parameters(std::string section_name) {
      pt::iptree used    = param_used.get_child(section_name);
      pt::iptree changed = param_changed.get_child(section_name);
      std::string key, value;
      int key_len, value_len;

      // Print the section name
      lout << std::endl << "[ " << section_name << " ]" << std::endl;

      // Sort the parameters by name
      used.sort();

      // Compute the lengths of the longest key and longest value (as string)
      key_len = 0;
      value_len = 0;
      for (pt::iptree::iterator pos = used.begin(); pos != used.end(); pos++) {
         key   = pos->first;
         value = used.get<std::string>(key);
         if (key.length() > key_len) {
            key_len = key.length();
         }
         if (value.length() > value_len) {
            value_len = value.length();
         }
      }

      // Print all the parameters
      for (pt::iptree::iterator pos = used.begin(); pos != used.end(); pos++) {
         key   = pos->first;
         value = used.get<std::string>(key);
         lout << "   " << std::left << std::setw(key_len) << key;
         lout << " : " << std::left << std::setw(value_len) << value;
         // Flag any that have been changed from default
         if (changed.get<bool>(key)) {
            lout << "   (CHANGED)";
         }
         lout << std::endl;
      }
   }

   // Print the parameters from the configuration file that were not used
   // - Inputs:
   //   - none
   // - Outputs:
   //   - none
   // - Side Effects:
   //   - none
   void print_unused_parameters() {
      
      pt::iptree::iterator outer, inner;
      pt::iptree child;

      // Don't do anything if the tree is empty
      if (!param_unused.empty()) {
         lout << std::endl << "Unused Parameters:" << std::endl;

         // Loop over all the sections in sorted order
         param_unused.sort();
         for (outer = param_unused.begin();
               outer != param_unused.end(); outer++) {

            // Get the subtree of all parameters in this section
            child = param_unused.get_child(outer->first);

            // Don't do anything if the subtree is empty
            if (!child.empty()) {

               // Print the section header
               lout << std::endl;
               lout << "[ " << outer->first << " ]" << std::endl;

               // Loop over all parameters in sorted order and print
               // - format is "   key : value" or "   key : value   (CHANGED)"
               child.sort();
               for (inner = child.begin(); inner != child.end(); inner++) {
                  lout << "   " << inner->first;
                  lout << " : " << child.get<std::string>(inner->first);
                  lout << std::endl;
               }
            }
         }
      }

      lout << std::endl;
   }

   // =========================================================================
   // Set up

   void setup (unsigned int argc, char *argv[]) {

      // ----------------------------------------------------------------------
      // Declare variables
      std::string param_file_name;
      bool changed_pf_name;

      // ----------------------------------------------------------------------
      // Arguments and parameters

      // Read the command-line arguments
      if (argc > 1) {
         param_file_name = argv[1];
         changed_pf_name = true;
      } else {
         param_file_name = "params.ini";
         changed_pf_name = false;
      }

      // Read the parameter file
      try {
         pt::read_ini(param_file_name, param_unused);
      } catch (pt::ini_parser_error &e) {
         std::cerr << "ERROR: Could not parse parameter file \"";
         std::cerr << param_file_name << "\"." << std::endl;
         throw;
      }

      // Add the command-line arguments to the parameter set
      param_used.put("Driver.config_file", param_file_name);
      param_changed.put("Driver.config_file", changed_pf_name);

      // ----------------------------------------------------------------------
      // Initialize the Driver component

      // Maximum number of time steps
      max_steps =
         parameter_with_default<unsigned int>("Driver.max_steps", 1000000);
      n_width = floor(log10(max_steps)) + 1;

      // Output directory
      output_dir =
         parameter_with_default<std::string>("Driver.output_dir", "output/");
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

      // Name of log file
      log_file =
         parameter_with_default<std::string>("Driver.log_file", "log.txt");
      log_file = output_dir + log_file;
      lout.open(log_file.c_str());

      // Time between saving output files
      output_dt = parameter_with_default<double>("Driver.output_dt", 0.0);

      // Restart from which directory
      restart_dir =
         parameter_with_default<std::string>("Driver.restart_dir", "");

      // Current time
      time = 0.0;

      // Final time
      tmax = parameter_without_default<double>("Driver.tmax");

      // Print the parameters for the Driver component
      lout << std::string(79, '_') << std::endl;
      lout << "Parameters:" << std::endl;

      print_parameters("Driver");

      // ----------------------------------------------------------------------
      // Initialize other components

      // Initialize the Grid component
      Grid::setup();
      print_parameters("Grid");

      // Initialize the Hydro component
      Hydro::setup();
      print_parameters("Hydro");

      // Initialize the InitConds (initial conditions) component
      InitConds::setup();
      print_parameters("InitConds");

      // Initialize the Monitor component
      Monitor::setup();
      print_parameters("Monitor");

      // Print the unused parameters
      print_unused_parameters();
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
      // Close the log file
      lout << std::endl << std::string(79, '_') << std::endl;
      lout << "Program complete" << std::endl;
      lout.close();
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

   void main (unsigned int argc, char *argv[]) {

      // ----------------------------------------------------------------------
      // Declare variables

      // Tracking output
      double prev_write = -1;
      double curr_write;
      bool do_write;
      std::string outname;

      // ----------------------------------------------------------------------
      // Initialize

      setup(argc, argv);

      // ----------------------------------------------------------------------
      // Evolution loop

      lout << std::string(79, '_') << std::endl;
      lout << "Evolution loop:" << std::endl;

      for (; n_step < max_steps; n_step++) {

         // Exceeded maximum time
         if (time >= tmax) {
            break;
         }

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
         // Conditions based on steps (print every output_steps seconds)
         // TODO
         // Do the actual write
         if (do_write) {
            outname = Grid::write_data();
            lout << "OUTPUT : wrote output " << outname << std::endl;
         }

         // Monitor write
         Monitor::write_to_monitor();

         // Boundary condition fill
         Grid::fill_boundary_conditions();

         // Compute step size
         dt = compute_time_step();

         // Print logfile note
         lout << "n = " << std::setw(n_width) << std::right << n_step;
         lout << "; t = " << std::setw(13) << std::scientific << time;
         lout << "; dt = " << std::setw(13) << std::scientific << dt;
         lout << std::endl;

         // Evolve a single step
         Hydro::one_step();

         // Update time
         time = time + dt;

      }

      // Final write
      lout << "n = " << std::setw(n_width) << std::right << n_step;
      lout << "; t = " << std::setw(13) << std::scientific << time;
      lout << std::endl;
      outname = Grid::write_data();
      lout << "OUTPUT : wrote output \"" << outname << "\"" << std::endl;
      Monitor::write_to_monitor();

      // ----------------------------------------------------------------------
      // Finalize

      cleanup();

   }

}
