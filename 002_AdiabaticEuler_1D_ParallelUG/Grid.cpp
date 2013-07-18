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
#include "Log.hpp"
#include "Parameters.hpp"
#include "Support.hpp"

namespace fs = boost::filesystem;

namespace Grid {

   // =========================================================================
   // component-scope variables

   // Number of guard (aka ghost) cells around the borders
   DelayedConst<unsigned int> Ng;

   // Should the guard cells be written?
   DelayedConst<bool> write_guard;

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

   // =========================================================================
   // Set up

   void setup () {

      // ----------------------------------------------------------------------
      // Get parameters

      // Number of guard cells
      Ng = fmax(1, Hydro::min_guard);

      // Should the guard cells be written?
      write_guard = Parameters::get_optional<bool>("Grid.write_guard", false);

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
      Log::write_single(std::string(79,'_') + "\n");
      Log::write_single("MPI Structure:\n\n");
      std::stringstream ss;
      ss << "Neighbors : < " << neigh_lo << " | " << Driver::proc_ID;
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
         x[i] = xmin + dx * (i + 0.5);
      }

      // Set up the grid
      data.init();

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
      double lo_recv[Ng.value()], hi_recv[Ng.value()];
      double lo_send[Ng.value()], hi_send[Ng.value()];
      int pass_up = 1;
      int pass_down = 2;
      int mpi_return;
      // Pack the send buffers
      for (int i = 0; i < Ng; i++) {
         try {
            lo_send[i] = data[ilo+i+Ng];
            hi_send[i] = data[ihi+i-Ng*2];
         } catch (...) {
            std::cerr << "Error packing send buffers";
            std::cerr << " in fill_boundary_conditions" << std::endl;
            throw;
         }
      }
      // Asynchronous receives
      MPI_Irecv(&lo_recv, Ng, MPI_DOUBLE, neigh_lo, pass_up,
            MPI_COMM_WORLD, &requests[0]);
      MPI_Irecv(&hi_recv, Ng, MPI_DOUBLE, neigh_hi, pass_down,
            MPI_COMM_WORLD, &requests[1]);
      // Asynchronous sends
      MPI_Isend(&lo_send, Ng, MPI_DOUBLE, neigh_lo, pass_down,
            MPI_COMM_WORLD, &requests[2]);
      MPI_Isend(&hi_send, Ng, MPI_DOUBLE, neigh_hi, pass_up,
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
         try{
            data[ilo+i   ] = lo_recv[i];
            data[ihi+i-Ng] = hi_recv[i];
         } catch (...) {
            std::cerr << "Error unpacking receive buffers";
            std::cerr << " in fill_boundary_conditions" << std::endl;
            throw;
         }
      }
#else // ifdef PARALLEL_MPI
      for (int i = 0; i < Ng; i++) {
         data[ilo   +i] = data[ihi-Ng*2+i];
         data[ihi-Ng+i] = data[ilo+Ng  +i];
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
      fout << "write_guard = " << write_guard << std::endl;
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
      fout << "# position" << std::endl << "# data" << std::endl;
      int ilim_lo;
      int ilim_hi;
      if (write_guard) {
         ilim_lo = ilo;
         ilim_hi = ihi;
      } else {
         ilim_lo = ilo+Ng;
         ilim_hi = ihi-Ng;
      }
      for (int i = ilim_lo; i < ilim_hi; i++) {
         fout << x[i] << "   " << data[i] << std::endl;
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
      std::vector<double> x_vec, data_vec;
      bool read_guard = false;

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
         } else if (line_key.find("write_guard") != std::string::npos) {
            ss.clear();
            ss.str(line_val);
            ss >> read_guard;
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

      int offset, Nread;
      if (read_guard) {
         offset = 0;
         Nread  = Nx_local + 2*Ng;
      } else {
         offset = Ng;
         Nread  = Nx_local;
      }
      if (x_vec.size() == Nread) {
         for (int i = 0; i < Nread; i++) {
            x[ilo+offset+i]    = x_vec[i];
            data[ilo+offset+i] = data_vec[i];
         }
      } else {
         std::cerr << Driver::proc_ID << " file contains " << x_vec.size();
         std::cerr << " cells" << std::endl;
         std::cerr << Driver::proc_ID << " code expects  " << Nread;
         std::cerr << " cells" << std::endl;
         throw std::length_error("length of file does not match Grid");
      }

   }

}
