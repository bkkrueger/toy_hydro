// STL includes
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>

// Boost includes

// Includes specific to this code
#include "Array.hpp"
#include "Driver.hpp"
#include "Grid.hpp"
#include "Hydro.hpp"
#include "Monitor.hpp"

namespace Monitor {

   // component-scope variables
   std::string monitor_file;
   std::ofstream mout;

#ifdef MONITOR_CONVERGENCE
   Array initial_data;
#endif // end ifdef MONITOR_CONVERGENCE

   // =========================================================================
   // Set up

   void setup () {

      // ----------------------------------------------------------------------
      // Get parameters

      monitor_file =
         Driver::parameter_with_default<std::string>("Monitor.monitor_file",
         "monitor.dat");
      monitor_file = Driver::output_dir + monitor_file;
      mout.open(monitor_file.c_str(), std::fstream::out | std::fstream::app);

#ifdef MONITOR_CONVERGENCE
      initial_data.resize(Grid::Nx);
      for (int i = 0; i < Grid::Nx; i++) {
         double temporary;
         try{
            temporary = Grid::data[i+Grid::Ng];
         } catch (std::out_of_range &e) {
            std::cerr << "error in getting initial data" << std::endl;
            throw;
         }
         try{
            initial_data[i] = temporary;
         } catch (std::out_of_range &e) {
            std::cerr << "error in saving initial data" << std::endl;
            std::cerr << "   " << initial_data.size() << " " << i << std::endl;
            throw;
         }
         //initial_data[i] = Grid::data[i+Grid::Ng];
      }
#endif // end ifdef MONITOR_CONVERGENCE

      // ----------------------------------------------------------------------
      // Write the header for the monitor file

      mout << std::endl << "#" << std::string(78, '=') << std::endl;
      mout << "# ";
      mout << std::setw(13) << "time";

#ifdef MONITOR_CONVERGENCE
      mout << "   " << std::setw(13) << "L1_norm";
      mout << "   " << std::setw(13) << "L2_norm";
      mout << "   " << std::setw(13) << "Linf_norm";
#endif // end ifdef MONITOR_CONVERGENCE

      mout << std::endl;

   }

#ifdef MONITOR_CONVERGENCE
   // =========================================================================
   // Shift initial solution to find expected solution at current time
   // (assuming periodic boundary conditions)

   double analytic_solution(double x) {

      // ----------------------------------------------------------------------
      // Declare variables
      double x_init, grid_width;
      double x1, y1, x2, y2;
      double y_x;

      // ----------------------------------------------------------------------
      // Compute shift for initial solution
      // --> If init state is q_i(x), solution at time t is q(x,t) = q_i(x-vt)
      x_init = x - Driver::time * Hydro::v_adv;
      grid_width = Grid::xmax - Grid::xmin;
      while (x_init < Grid::xmin) {
         x_init += grid_width;
      }
      while (Grid::xmax < x_init) {
         x_init -= grid_width;
      }

      // ----------------------------------------------------------------------
      // Interpolate initial solution at x_init
      if ((x_init < Grid::x[Grid::Ng]) ||
            (Grid::x[Grid::Ng+Grid::Nx-1] < x_init)) {
         // interpolate between topmost point and lowermost point
         x1 = Grid::x[Grid::Ng];
         y1 = initial_data[0];
         x2 = Grid::x[Grid::Ng+Grid::Nx-1];
         y2 = initial_data[Grid::Nx-1];
      } else {
         int i = Grid::Ng + 1;
         while (Grid::x[i] < x_init) {
            i++;
         }
         // interpolate between x[i-1] and x[i]
         x1 = Grid::x[i-1];
         y1 = initial_data[i-1-Grid::Ng];
         x2 = Grid::x[i];
         y2 = initial_data[i-Grid::Ng];
      }
      y_x = (y2-y1)*(x_init-x1)/(x2-x1) + y1;

      return y_x;

   }
#endif // end ifdef MONITOR_CONVERGENCE

   // =========================================================================
   // Compute quantities to be monitored

   void write_to_monitor() {

      // ----------------------------------------------------------------------
      // Compute quantities to be monitored

      mout << "  " << std::setw(13) << std::scientific << Driver::time;

#ifdef MONITOR_CONVERGENCE
      double L1 = 0;
      double L2 = 0;
      double Linf = 0;
      double resid;
      for (int i = Grid::Ng; i < Grid::Ng+Grid::Nx; i++) {
         try{
         resid = std::abs(Grid::data[i] - analytic_solution(Grid::x[i]));
         } catch (std::out_of_range &e) {
            std::cerr << "error in analytic_solution" << std::endl;
            throw;
         }
         L1 += resid;
         L2 += std::pow(resid, 2);
         if (resid > Linf) {
            Linf = resid;
         }
      }
      L2 = std::sqrt(L2);
      mout << "   " << std::setw(13) << std::scientific << L1;
      mout << "   " << std::setw(13) << std::scientific << L2;
      mout << "   " << std::setw(13) << std::scientific << Linf;
#endif // end ifdef MONITOR_CONVERGENCE

      mout << std::endl;

   }

   // =========================================================================
   // Clean up

   void cleanup() {
      mout.close();
   }

}
