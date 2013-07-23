#include "Defines.hpp"

// STL includes
#include <cmath>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

// Boost includes
#include <boost/filesystem.hpp>

// Other 3rd-party includes
#ifdef PARALLEL_MPI
#include "mpi.h"
#endif // end ifdef PARALLEL_MPI

// Includes specific to this code
#include "Driver.hpp"
#include "Grid.hpp"
#include "GridVars.hpp"
#include "Hydro.hpp"
#include "InitConds.hpp"
#include "Log.hpp"
#include "Parameters.hpp"
#include "Support.hpp"

namespace fs = boost::filesystem;

namespace Grid {

   // =========================================================================
   // component-scope variables

   // Number of guard (aka ghost) cells around the borders
   DelayedConst<unsigned int> Ng;

   // Number of internal (non-guard) cells
   DelayedConst<unsigned int> Nx_global; // globally
   DelayedConst<unsigned int> Nx_local;  // on this processor

   // Upper and lower limits
   DelayedConst<double> xmin, xmax;

   // Grid spacing
   DelayedConst<double> dx;

   // Array of coordinates
   CellVar x;

   // Data grid
   CellVar data;

   // Indices of arrays
   DelayedConst<int> ilo, ihi; // arrays include indices ilo to ihi-1

   // The processor IDs of the lower and upper neighbors
   DelayedConst<int> neigh_lo, neigh_hi;

   std::vector<std::string> var_list;
   DelayedConst<unsigned int> n_vars;

   // Output precision
   const unsigned int w = 30;

   // =========================================================================
   // Add a new variable to the Grid

   unsigned int add_variable(std::string new_var) {
      for (unsigned int i = 0; i < var_list.size(); i++) {
         if (var_list[i] == new_var) {
            return i;
         }
      }
      var_list.push_back(new_var);
      return var_list.size() - 1;
   }

   // =========================================================================
   // Set up

   void setup () {

      std::stringstream ss;

      Log::write_single(std::string(79,'_') + "\n");
      Log::write_single("Grid Setup:\n\n");

      // ----------------------------------------------------------------------
      // Get parameters

      // Number of guard cells
      Ng = fmax(1, Hydro::min_guard);

      // Number of internal cells
      Nx_global = Parameters::get_required<unsigned int>("Grid.Nx");

      // Limits
      xmin = Parameters::get_required<double>("Grid.xmin");
      xmax = Parameters::get_required<double>("Grid.xmax");

      // ----------------------------------------------------------------------
      // Set up grid

      // Coordinates
      dx = (xmax - xmin) / Nx_global;

#ifdef PARALLEL_MPI
      // Determine neighbors
      if (Driver::proc_ID == 0) {
         neigh_lo = Driver::n_procs - 1;
      } else {
         neigh_lo = Driver::proc_ID - 1;
      }
      if (Driver::proc_ID == Driver::n_procs - 1) {
         neigh_hi = 0;
      } else {
         neigh_hi = Driver::proc_ID + 1;
      }
      // Print a summary of neighbors
      ss << "MPI Neighbors : < " << neigh_lo << " | " << Driver::proc_ID;
      ss << " | " << neigh_hi << " >" << std::endl;
      Log::write_all(ss.str());
      Log::write_single("\n");

      // Compute limits
      ilo = (Nx_global *  Driver::proc_ID   ) / Driver::n_procs - Ng;
      ihi = (Nx_global * (Driver::proc_ID+1)) / Driver::n_procs + Ng;
      // Compute size
      Nx_local = ihi - ilo - 2*Ng;
#else // PARALLEL_MPI
      // Compute limits
      ilo = 0         - Ng;
      ihi = Nx_global + Ng;
      // Compute size
      Nx_local = Nx_global;
#endif // end ifdef PARALLEL_MPI
      // Verify Nx_local >= Ng or the communication becomes absurd
      if (Nx_local < Ng) {
         throw std::length_error("The number of local internal cells must exceed the number of guard cells");
      }
      // Fill the coordinates
      x.init();
      for (int i = ilo; i < ihi; i++) {
         x(i) = xmin + dx * (i + 0.5);
      }

      // Set up the grid
      Hydro::add_variables();
      InitConds::add_variables();
      /* Add add_variables() for any other components that want variables. */
      n_vars = var_list.size();
      data.init(n_vars);

      ss.clear();
      ss.str("");
      ss << "Simulating with " << n_vars << " variables:" << std::endl;
      Log::write_single(ss.str());
      unsigned int v_width = fmax(floor(log10(n_vars)) + 1, 2);
      for (unsigned int v = 0; v < n_vars; v++) {
         ss.clear();
         ss.str("");
         ss << "  ";
         ss << std::setfill('0') << std::setw(v_width) << v;
         ss << " : " << var_list[v] << std::endl;
         Log::write_single(ss.str());
      }
      Log::write_single("\n");
   }

   // =========================================================================
   // Clean up

   void cleanup () {
      // Does nothing; the grids will call their destructors when they go out
      // of scope, and nothing else needs to be done here.
   }

   // =========================================================================
   // Fill boundary conditions

   void fill_boundary_conditions() {
#ifdef PARALLEL_MPI
      // Declare some variables
      MPI_Request requests[4];   // Two sends and two receives (1 up, 1 down)
      MPI_Status  statuses[4];   // Statuses of sends/receives
      unsigned int n_trans = Ng * n_vars;
      double lo_recv[n_trans], hi_recv[n_trans];
      double lo_send[n_trans], hi_send[n_trans];
      int pass_up = 1;
      int pass_down = 2;
      int mpi_return;
      // Pack the send buffers
      for (int i = 0; i < Ng; i++) {
         for (unsigned int v = 0; v < n_vars; v++) {
            try {
               lo_send[i*n_vars+v] = data(ilo+i+Ng,  v);
               hi_send[i*n_vars+v] = data(ihi+i-Ng*2,v);
            } catch (...) {
               std::cerr << "Error packing send buffers";
               std::cerr << " in fill_boundary_conditions" << std::endl;
               throw;
            }
         }
      }
      // Asynchronous receives
      MPI_Irecv(&lo_recv, n_trans, MPI_DOUBLE, neigh_lo, pass_up,
            MPI_COMM_WORLD, &requests[0]);
      MPI_Irecv(&hi_recv, n_trans, MPI_DOUBLE, neigh_hi, pass_down,
            MPI_COMM_WORLD, &requests[1]);
      // Asynchronous sends
      MPI_Isend(&lo_send, n_trans, MPI_DOUBLE, neigh_lo, pass_down,
            MPI_COMM_WORLD, &requests[2]);
      MPI_Isend(&hi_send, n_trans, MPI_DOUBLE, neigh_hi, pass_up,
            MPI_COMM_WORLD, &requests[3]);
      // Wait for sends and receives to finish
      mpi_return = MPI_Waitall(4, requests, statuses);
      if (mpi_return != MPI_SUCCESS) {
         std::cerr << "boundary condition/guard cell fill failed";
         std::cerr << std::endl;
         MPI_Abort(MPI_COMM_WORLD, mpi_return);
      }
      // Unpack receive buffers
      for (int i = 0; i < Ng; i++) {
         for (unsigned int v = 0; v < n_vars; v++) {
            try{
               data(ilo+i   ,v) = lo_recv[i*n_vars+v];
               data(ihi+i-Ng,v) = hi_recv[i*n_vars+v];
            } catch (...) {
               std::cerr << "Error unpacking receive buffers";
               std::cerr << " in fill_boundary_conditions" << std::endl;
               throw;
            }
         }
      }
#else // ifdef PARALLEL_MPI
      for (int i = 0; i < Ng; i++) {
         for (unsigned int v = 0; v < n_vars; v++) {
            data(ilo   +i,v) = data(ihi-Ng*2+i,v);
            data(ihi-Ng+i,v) = data(ilo+Ng  +i,v);
         }
      }
#endif // ifdef PARALLEL_MPI
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
      ss.clear();
      ss.str("");
      ss << std::setfill('0') << std::setw(Driver::n_width) << Driver::n_step;
      ss >> dirname;
      dirname = Driver::output_dir + "step_" + dirname;
#ifdef PARALLEL_MPI
      if (Driver::proc_ID == 0) {
#endif // ifdef PARALLEL_MPI
         if (fs::exists(dirname)) { // Remove existing folder with same name
            fs::remove_all(fs::path(dirname));
         }
         fs::create_directories(dirname);    // Create the new directory
#ifdef PARALLEL_MPI
      }
      // Sync to ensure directory exist before writing begins
      MPI_Barrier(MPI_COMM_WORLD);
#endif // ifdef PARALLEL_MPI

      // Write the important header information
      filename = dirname + "/header.txt";
      fout.open(filename.c_str());
      fout << "time        = " << Driver::time << std::endl;
      fout << "step        = " << Driver::n_step << std::endl;
      fout.close();

      // Write the data
#ifdef PARALLEL_MPI
      ss.str("");
      ss.clear();
      ss << std::setfill('0') << std::setw(Driver::p_width) << Driver::proc_ID;
      ss >> filename;
      filename = dirname + "/grid_" + filename + ".dat";
#else // PARALLEL_MPI
      filename = dirname + "/grid.dat";
#endif // PARALLEL_MPI
      fout.open(filename.c_str());
      fout << "# position" << std::endl;
      for (unsigned int v = 0; v < n_vars; v++) {
         fout << "# " << var_list[v] << std::endl;
      }
      fout.precision(w-8);
      fout.setf(std::ios::scientific);
      for (int i = ilo+Ng; i < ihi-Ng; i++) {
         fout << std::setw(w) << x(i);
         for (unsigned int v = 0; v < n_vars; v++) {
            fout << "   " << std::setw(w) << data(i,v);
         }
         fout << std::endl;
      }
      fout.close();

      return dirname;

   }

   // =========================================================================
   // Load data from a file

   // TODO --- allow redistribution to a different number of cores
   void read_data() {

      // Declare variables ----------------------------------------------------

      std::size_t pos;
      std::string filename, line;
      std::string line_key, line_val;
      std::ifstream fin;
      std::stringstream ss;
      int i;
      double xin, din;
      std::vector< std::vector<double> > data_vec;
      std::vector<double> x_vec, data_row;
      std::vector<unsigned int> idx_map(n_vars, -1);
      unsigned int var_idx = 0;

      // Process the header file ----------------------------------------------

      // Make sure the file exists
      filename = Driver::restart_dir + "header.txt";
      if (!fs::exists(filename)) {
         throw std::ios_base::failure("Could not find header file.");
      }

      // Get the important info from the file
      fin.open(filename.c_str());
      while (std::getline(fin, line)) {
         pos = line.find("=") + 1;
         line_key = line.substr(0,pos);
         line_val = line.substr(pos);
         if (line_key.find("time") != std::string::npos) {
            ss.clear();
            ss.str(line_val);
            ss >> Driver::time;
         } else if (line_key.find("step") != std::string::npos) {
            ss.clear();
            ss.str(line_val);
            ss >> Driver::n_step;
         }
      }
      fin.close();

      // Make a note in the log file about restarting
      ss.clear();
      ss.str("");
      ss << "\nRestarting from step " << Driver::n_step;
      ss << " and time " << Driver::time << ".\n\n";
      Log::write_single(std::string(79,'_')+"\n");
      Log::write_single(ss.str());

      // Load the data file ---------------------------------------------------

      // Get the name of the data file
#ifdef PARALLEL_MPI
      // Loop over all files in restart_dir to find one that matches
      // "grid_#.dat" with the number matching proc_ID
      // --> Have to loop in case the number of extra zeros padded onto the
      //     front of the processor ID is not the same now as when the files
      //     were written.
      fs::directory_iterator end;   // no argument to constructor => end iter
      std::string path, num_str;
      int num_int;
      bool file_not_found = true;
      for (fs::directory_iterator iter(Driver::restart_dir);
            iter != end; iter++) {
         path = (*iter).path().string();
         path = path.substr(path.find_last_of("/")+1);
         if (path.substr(0,5) != "grid_") {
            continue;
         }
         num_str = path.substr(5,path.find_last_of(".")-5);
         std::stringstream ss(num_str);
         if ((ss >> num_int) && (num_int == Driver::proc_ID)) {
            filename = Driver::restart_dir + "grid_" + num_str + ".dat";
            file_not_found = false;
            break;
         }
      }
      if (file_not_found) {
         throw std::ios_base::failure("Could not find data file.");
      }
#else // PARALLEL_MPI
      filename = Driver::restart_dir + "/grid.dat";
      if (!fs::exists(filename)) {
         throw std::ios_base::failure("Could not find data file.");
      }
#endif // PARALLEL_MPI

      // Open the data file
      fin.open(filename.c_str());

      // Process the data file
      var_idx = 0;
      while (std::getline(fin, line)) {
         pos = line.find('#');
         if (pos != std::string::npos) {
            // Comment line
            line = line.substr(pos+2);
            for (unsigned int v = 0; v < n_vars; v++) {
               if (var_list[v] == line) {
                  idx_map[v] = var_idx;
                  var_idx++;
                  break;
               }
            }
         } else {
            // Data line
            std::istringstream iss(line);
            iss >> xin;
            while (iss >> din) {
               data_row.push_back(din);
            }
            x_vec.push_back(xin);
            data_vec.push_back(data_row);
            data_row.clear();
         }
      }

      // Close the data file
      fin.close();

      // Store to Grid --------------------------------------------------------

      if (x_vec.size() == Nx_local) {
         for (int i = 0; i < Nx_local; i++) {
            x(ilo+Ng+i)    = x_vec[i];
            if (data_vec[i].size() == n_vars) {
               for (unsigned int v = 0; v < n_vars; v++) {
                  data(ilo+Ng+i,idx_map[v]) = data_vec[i][v];
               }
            } else {
               std::stringstream ss;
               ss << "not enough values for cell " << i << std::endl;
               throw std::length_error(ss.str());
            }
         }
      } else {
         std::cerr << Driver::proc_ID << " file contains " << x_vec.size();
         std::cerr << " cells" << std::endl;
         std::cerr << Driver::proc_ID << " code expects  " << Nx_local;
         std::cerr << " cells" << std::endl;
         throw std::length_error("length of file does not match Grid");
      }

   }

}
