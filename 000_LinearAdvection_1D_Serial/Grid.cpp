// TODO : Do I want to turn this into a class to allow public/private?  There
// are also unnamed namespaces, but I don't know the usage of those so I'm not
// yet confident enough to use them myself.

// STL includes
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

// Boost includes
#include <boost/filesystem.hpp>

// Includes specific to this code
#include "Array.hpp"
#include "Driver.hpp"
#include "Grid.hpp"

namespace fs = boost::filesystem;

namespace Grid {

   // component-scope variables
   unsigned int Ng;     // the number of guard cells around the borders

   unsigned int Nx;     // number of internal (non-guard) cells in x direction
   double xmin, xmax;   // limits in the x direction
   double dx;           // grid spacing in the x direction

   Array x;             // array of x coordinates
   Array data;          // the data grid

   // =========================================================================
   // Set up

   void setup () {

      // ----------------------------------------------------------------------
      // Declare variables

      // ----------------------------------------------------------------------
      // Get parameters

      // Number of guard cells
      Ng = 1;

      // Number of internal cells
      Nx = Driver::parameter_without_default<unsigned int>("Grid.Nx");

      // Limits
      xmin = Driver::parameter_without_default<double>("Grid.xmin");
      xmax = Driver::parameter_without_default<double>("Grid.xmax");

      // ----------------------------------------------------------------------
      // Set up grid

      // Coordinates
      dx = (xmax - xmin) / Nx;
      x.resize(Nx+2*Ng);
      for (unsigned int i = 0; i < x.size(); i++) {
         x[i] = xmin + dx * (i + 0.5 - Ng);
      }

      // Set up the grid
      data.resize(Nx+2*Ng);

   }

   // =========================================================================
   // Clean up

   void cleanup () {
      x.resize(0);
      data.resize(0);
   }

   // =========================================================================
   // Fill boundary conditions

   void fill_boundary_conditions() {
      for (unsigned int i = 0; i < Ng; i++) {
         data[i] = data[Nx+i];
         data[Ng+Nx+i] = data[Ng+i];
      }
   }

   // =========================================================================
   // Write the data to a file

   std::string write_data () {

      // ----------------------------------------------------------------------
      // Declare variables

      std::stringstream ss;
      std::string dirname, filename;
      std::ofstream fout;

      // ----------------------------------------------------------------------
      // Write the output

      // Create the subdirectory for the current output
      ss.str("");
      ss.clear();
      ss << std::setfill('0') << std::setw(Driver::n_width) << Driver::n_step;
      ss >> dirname;
      dirname = Driver::output_dir + "step_" + dirname;
      if (!fs::exists(dirname)) {
         fs::create_directories(dirname);
      }

      // Write the important header information
      filename = dirname + "/header.txt";
      fout.open(filename.c_str());
      fout << "time = " << Driver::time << std::endl;
      fout << "step = " << Driver::n_step << std::endl;
      fout.close();

      // Write the data
      filename = dirname + "/grid.dat";
      fout.open(filename.c_str());
      fout << "# position" << std::endl << "# data" << std::endl;
      for (unsigned int i = Ng; i < Ng+Nx; i++) {
         fout << x[i] << "   " << data[i] << std::endl;
      }
      fout.close();

      return dirname;

   }

   // =========================================================================
   // Load data from a file

   void read_data() {

      // Declare variables ----------------------------------------------------

      std::size_t pos;
      std::string filename, line, junk_string;
      std::ifstream fin;
      int i;
      double xin, din;
      std::vector<double> x_vec, data_vec;

      // Process the header file ----------------------------------------------

      filename = Driver::restart_dir + "/header.txt";
      fin.open(filename.c_str());
      while (std::getline(fin, line)) {
         pos = line.find("=") + 1;
         if (line.find("time =") != std::string::npos) {
            std::istringstream iss(line.substr(pos));
            iss >> Driver::time;
         } else if (line.find("step =") != std::string::npos) {
            std::istringstream iss(line.substr(pos));
            iss >> Driver::n_step;
         }
      }
      fin.close();

      // Load the data file ---------------------------------------------------

      filename = Driver::restart_dir + "/grid.dat";
      fin.open(filename.c_str());
      while (std::getline(fin, line)) {
         if (line.find('#') == std::string::npos) {
            std::istringstream iss(line);
            iss >> xin >> din;
            x_vec.push_back(xin);
            data_vec.push_back(din);
         }
      }
      fin.close();

      // Store to Grid --------------------------------------------------------

      if (x_vec.size() == Nx) {
         for (int i = 0; i < Nx; i++) {
            x[Ng+i]    = x_vec[i];
            data[Ng+i] = data_vec[i];
         }
      } else {
         throw std::length_error("length of file does not match Grid");
      }

   }

}
