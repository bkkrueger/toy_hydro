#include "Defines.hpp"

// STL includes
#include <iomanip>
#include <string>

// Boost includes
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

// Other 3rd-party includes

// Includes specific to this code
#include "Driver.hpp"
#include "Log.hpp"

namespace pt = boost::property_tree;

namespace Parameters {

   // parameter listings (all are case-insensitive PropertyTrees from Boost)
   pt::iptree param_used;     // parameters used by the code
   pt::iptree param_unused;   // parameters not used by the code
   pt::iptree param_changed;  // have parameters changed from default?

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
      std::stringstream ss;

      // Print the section name
      ss << std::endl << "[ " << section_name << " ]" << std::endl;
      Log::write_single(ss.str());

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
         ss.clear();
         ss.str("");
         ss << "   " << std::left << std::setw(key_len) << key;
         ss << " : " << std::left << std::setw(value_len) << value;
         Log::write_single(ss.str());
         // Flag any that have been changed from default
         if (changed.get<bool>(key)) {
            Log::write_single("   (CHANGED)");
         }
         Log::write_single("\n");
      }
   }

   void print_used_parameters() {
      
      pt::iptree::iterator outer, inner;
      pt::iptree child;
      std::stringstream ss;

      // Don't do anything if the tree is empty
      if (!param_used.empty()) {
         Log::write_single(std::string(79,'_')+"\n");
         Log::write_single("Parameters:\n");

         // Loop over all the sections in sorted order
         param_used.sort();
         for (outer = param_used.begin(); outer != param_used.end(); outer++) {
            print_parameters(outer->first);
         }
      }

      Log::write_single("\n");

   }

   // =========================================================================
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
      std::stringstream ss;

      // Don't do anything if the tree is empty
      if (!param_unused.empty()) {
         Log::write_single(std::string(79,'_')+"\n");
         Log::write_single("Unused Parameters:\n");

         // Loop over all the sections in sorted order
         param_unused.sort();
         for (outer = param_unused.begin();
               outer != param_unused.end(); outer++) {

            // Get the subtree of all parameters in this section
            child = param_unused.get_child(outer->first);

            // Don't do anything if the subtree is empty
            if (!child.empty()) {

               // Print the section header
               ss.clear();
               ss.str("");
               ss << std::endl;
               ss << "[ " << outer->first << " ]" << std::endl;
               Log::write_single(ss.str());

               // Loop over all parameters in sorted order and print
               // - format is "   key : value" or "   key : value   (CHANGED)"
               child.sort();
               for (inner = child.begin(); inner != child.end(); inner++) {
                  ss.clear();
                  ss.str("");
                  ss << "   " << inner->first;
                  ss << " : " << child.get<std::string>(inner->first);
                  ss << std::endl;
                  Log::write_single(ss.str());
               }
            }
         }
      }

      Log::write_single("\n");
   }

   // =========================================================================
   // Set up

   void setup (int argc, char *argv[]) {

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

   }

   // =========================================================================
   // Clean up

   void cleanup () {
   }

}
