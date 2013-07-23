#include "Defines.hpp"

// STL includes
#include <cmath>

// Boost includes

// Includes specific to this code
#include "Driver.hpp"
#include "Grid.hpp"
#include "Log.hpp"
#include "Parameters.hpp"

namespace InitConds {

   // component-scope variables
   double x0, dx;
   double y0, dy;

   // Variable indices
   DelayedConst<unsigned int> idx_g, idx_p, idx_s;

   // =========================================================================
   // Variable request list

   void add_variables () {
      idx_g = Grid::add_variable("gaussian");
      idx_p = Grid::add_variable("parabola");
      idx_s = Grid::add_variable("step_fxn");
   }

   // =========================================================================
   // Set up

   void setup () {

      // ----------------------------------------------------------------------
      // Declare variables

      double temp;

      // ----------------------------------------------------------------------
      // Initialize the InitConds component

      Log::write_single(std::string(79,'_') + "\n");
      Log::write_single("InitConds Setup:\n\n");

      x0 = Parameters::get_optional<double>("InitConds.x0", 0.00);
      dx = Parameters::get_optional<double>("InitConds.dx", 0.75);
      y0 = Parameters::get_optional<double>("InitConds.y0", 10.0);
      dy = Parameters::get_optional<double>("InitConds.dy", 1.25);

      // ----------------------------------------------------------------------
      // Set the initial data

      if (Driver::restart_dir == "") {
         for (int i = Grid::ilo; i < Grid::ihi; i++) {
            temp = (Grid::x(i) - x0) / dx;
            temp = temp * temp;
            Grid::data(i,idx_g) = y0 + dy * exp(-temp);
            Grid::data(i,idx_p) = y0 + dy * fmax(0.0, 1.0 - temp);
            Grid::data(i,idx_s) = y0;
            if (sqrt(temp) <= 1.0) {
               Grid::data(i,idx_s) += dy;
            }
         }
      } else {
         try{
            Grid::read_data();
         } catch (...) {
            throw;
         }
      }

   }

   // =========================================================================
   // Clean up

   void cleanup () {
   }

}
