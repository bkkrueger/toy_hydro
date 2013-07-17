#include "Defines.hpp"

// STL includes
#include <cmath>
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
#include "Log.hpp"
#include "Parameters.hpp"
#include "Support.hpp"

namespace fs = boost::filesystem;

namespace Grid {

   // component-scope variables
   DelayedConst<unsigned int> Ng; // number of guard cells around the borders

   // number of internal (non-guard) cells
   DelayedConst<unsigned int> Nx_global; // globally
   DelayedConst<unsigned int> Nx_local;  // on this processor

   DelayedConst<double> xmin, xmax;   // limits in the x direction
   DelayedConst<double> dx;           // grid spacing in the x direction

   CellVar x;             // array of x coordinates
   CellVar data;          // the data grid
   DelayedConst<int> ilo, ihi; // arrays include indices ilo to ihi-1

   // =========================================================================
   // Set up

   void setup () {

      // ----------------------------------------------------------------------
      // Declare variables

      std::stringstream ss;

      // ----------------------------------------------------------------------
      // Get parameters

      // Number of guard cells
      Ng = 1;  // TODO - Grid is initialized before Hydro, but Hydro may have
               // to insist on a minimum Ng --- perhaps have some const in
               // Hydro that Grid reads and increases Ng if necessary?

      // Number of internal cells
      Nx_global =
         Parameters::parameter_without_default<unsigned int>("Grid.Nx");

      // Limits
      xmin = Parameters::parameter_without_default<double>("Grid.xmin");
      xmax = Parameters::parameter_without_default<double>("Grid.xmax");

      // ----------------------------------------------------------------------
      // Set up grid

      // Coordinates
      dx = (xmax - xmin) / Nx_global;
#ifdef PARALLEL_MPI
      ilo = (Nx_global *  Driver::proc_ID   ) / Driver::n_procs - Ng;
      ihi = (Nx_global * (Driver::proc_ID+1)) / Driver::n_procs + Ng;
      Nx_local = ihi - ilo - 2*Ng;
      ss << "Processor " << Driver::proc_ID << " : ";
      ss << "i = <" << ilo << " (" << ilo+Ng;
      ss << "," << ihi-Ng-1 << ") " << ihi-1 << ">";
      ss << std::endl;
      Log::write_all(ss.str());
      Log::write_single("\n");
#else // PARALLEL_MPI
      ilo = 0         - Ng;
      ihi = Nx_global + Ng;
      Nx_local = Nx_global;
#endif // end ifdef PARALLEL_MPI
      // TODO : I should make sure that Nx_local > Ng
      x.init();
      for (int i = ilo; i < ihi; i++) {
         x[i] = xmin + dx * (i + 0.5);
      }

#ifdef PARALLEL_MPI
      MPI_Barrier(MPI_COMM_WORLD); // to avoid conflict with previous write
      ss.clear();
      ss.str("");
      ss << "Processor " << Driver::proc_ID << " : ";
      ss << "x = <" << x[ilo] << " (" << x[ilo+Ng];
      ss << "," << x[ihi-Ng-1] << ") " << x[ihi-1] << ">";
      ss << std::endl;
      Log::write_all(ss.str());
      Log::write_single("\n");
#endif // PARALLEL_MPI


      // Set up the grid
      data.init();

   }

   // =========================================================================
   // Clean up

   void cleanup () {
      // Does nothing
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
      MPI_Irecv(&lo_recv, Ng, MPI_DOUBLE, Driver::neigh_lo, pass_up,
            MPI_COMM_WORLD, &requests[0]);
      MPI_Irecv(&hi_recv, Ng, MPI_DOUBLE, Driver::neigh_hi, pass_down,
            MPI_COMM_WORLD, &requests[1]);
      // Asynchronous sends
      MPI_Isend(&lo_send, Ng, MPI_DOUBLE, Driver::neigh_lo, pass_down,
            MPI_COMM_WORLD, &requests[2]);
      MPI_Isend(&hi_send, Ng, MPI_DOUBLE, Driver::neigh_hi, pass_up,
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
      ss.str("");
      ss.clear();
      ss << std::setfill('0') << std::setw(Driver::n_width) << Driver::n_step;
      ss >> dirname;
      dirname = Driver::output_dir + "step_" + dirname;
      MPI_Barrier(MPI_COMM_WORLD);
      if (Driver::proc_ID == 0) {
         if (fs::exists(dirname)) { // Remove existing folder with same name
            fs::remove_all(fs::path(dirname));
         }
      }
      MPI_Barrier(MPI_COMM_WORLD);
      fs::create_directories(dirname);    // Create the new directory

      // Write the important header information
      filename = dirname + "/header.txt";
      fout.open(filename.c_str());
      fout << "time = " << Driver::time << std::endl;
      fout << "step = " << Driver::n_step << std::endl;
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
      //for (int i = ilo+Ng; i < ihi-Ng; i++) { // TODO - print guard optional
      for (int i = ilo; i < ihi; i++) {
         fout << x[i] << "   " << data[i] << std::endl;
      }
      fout.close();

      return dirname;

   }

   // =========================================================================
   // Load data from a file

   // TODO --- redistribute to a different number of cores
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

#ifdef PARALLEL_MPI
      // Loop over all files in restart_dir to find one that matches
      // "grid_#.dat" with the number matching proc_ID
      fs::directory_iterator end;   // no argument to constructor => end iter
      std::string path, num_str;
      int num_int;
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
            filename = Driver::restart_dir + "/grid_" + num_str + ".dat";
            break;
         }
      }
#else // PARALLEL_MPI
      filename = Driver::restart_dir + "/grid.dat";
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

      /*if (x_vec.size() == Nx_local) {   // TODO - check if write_guard or not
         for (int i = 0; i < Nx_local; i++) {
            x[ilo+Ng+i]    = x_vec[i];
            data[ilo+Ng+i] = data_vec[i];
         }*/
      if (x_vec.size() == Nx_local+2*Ng) {
         for (int i = 0; i < Nx_local+2*Ng; i++) {
            x[ilo+i]    = x_vec[i];
            data[ilo+i] = data_vec[i];
         }
      } else {
         std::cerr << Driver::proc_ID << " file has " << x_vec.size() << " cells" << std::endl;
         //std::cerr << Driver::proc_ID << " code has " << Nx_local     << " cells" << std::endl; // TODO - this is only for testing - formalize or delete
         std::cerr << Driver::proc_ID << " code has " << Nx_local+2*Ng<< " cells" << std::endl;
         throw std::length_error("length of file does not match Grid");
      }

   }

}
