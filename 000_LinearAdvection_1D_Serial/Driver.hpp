#ifndef DRIVER_HPP
#define DRIVER_HPP

// TODO : Do I want to turn this into a class to allow public/private?  There
// are also unnamed namespaces, but I don't know the usage of those so I'm not
// yet confident enough to use them myself.

// STL includes
#include <cmath>
#include <fstream>
#include <string>

// Boost includes
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

// Includes specific to this code
#include "Grid.hpp"
#include "Hydro.hpp"
#include "InitConds.hpp"

namespace pt = boost::property_tree;

namespace Driver {

   // parameter listings (all are case-insensitive PropertyTrees from Boost)
   extern pt::iptree param_used;     // parameters used by the code
   extern pt::iptree param_unused;   // parameters not used by the code
   extern pt::iptree param_changed;  // have parameters changed from default?

   // component-scope variables
   extern unsigned int n_step;
   extern unsigned int max_steps;
   extern unsigned int n_width;

   extern double dt;
   extern double time;
   extern double tmax;

   extern double output_dt;

   extern std::string output_dir;
   extern std::string restart_dir;
   extern std::ofstream lout;

   // =========================================================================
   // Accessing parameters from the ptree; other components should use
   // parameter_with_default and parameter_without_default instead of
   // param_found or param_default.

   // Marks a parameter as found (it exists in the configuration file and is
   // being used by the code)
   template <class T>
   void param_found(std::string param_name, T param_value) {
      // Put it in the list of used parameters
      param_used.put(param_name, param_value);

      // Mark it as having been changed from the default value
      param_changed.put(param_name, true);

      // Erase it from the list of unused parameters
      size_t n = param_name.find('.');
      if (n == std::string::npos) {
         param_unused.erase(param_name);
      } else {
         std::string section = param_name.substr(0,n);
         std::string parameter = param_name.substr(n+1);
         param_unused.get_child(section).erase(parameter);
      }
   }

   // Marks a parameter as using the default
   template <class T>
   void param_default(std::string param_name, T param_value) {
      // Put it in the list of used parameters
      param_used.put(param_name, param_value);

      // Mark it as still having the default value (unchanged)
      param_changed.put(param_name, false);
   }

   // Fetch a parameter that has a default value
   template <class T>
   T parameter_with_default(std::string param_name, T default_value) {
      T value;
      try {
         value = param_unused.get<T>(param_name);
         param_found(param_name, value);
      } catch (pt::ptree_bad_path &e) {
         // Parameter isn't there --- use the default
         value = default_value;
         param_default(param_name, value);
      } catch (pt::ptree_bad_data &e) {
         // Parameter cannot be converted to desired type
         std::cerr << "ERROR: Bad data for parameter \"" << param_name;
         std::cerr << "\"." << std::endl;
         throw;
      } catch (...) {
         throw;
      }
      return value;
   }

   // Fetch a parameter that has no default value (required parameter)
   template <class T>
   T parameter_without_default(std::string param_name) {
      T value;
      try {
         value = param_unused.get<T>(param_name);
         param_found(param_name, value);
      } catch (pt::ptree_bad_path &e) {
         // Parameter isn't there, but is required
         std::cerr << "ERROR: Parameter \"" << param_name << "\" is required.";
         std::cerr << std::endl;
         throw;
      } catch (pt::ptree_bad_data &e) {
         std::cerr << "ERROR: Bad data for parameter \"" << param_name;
         std::cerr << "\"." << std::endl;
         throw;
      } catch (...) {
         throw;
      }
      return value;
   }

   // =========================================================================
   // Print the parameters for the specified section

   void print_parameters(std::string section_name);

   // =========================================================================
   // Set up

   void setup (unsigned int argc, char *argv[]);

   // =========================================================================
   // Clean up

   void cleanup ();

   // =========================================================================
   // Compute the time step

   double compute_time_step();

   // =========================================================================
   // The main evolution loop

   void main (unsigned int argc, char *argv[]);

}

#endif
