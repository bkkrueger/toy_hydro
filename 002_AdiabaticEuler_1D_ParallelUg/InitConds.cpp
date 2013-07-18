#include "Defines.hpp"

// STL includes
#include <cmath>

// Boost includes

// Includes specific to this code
#include "Driver.hpp"
#include "Grid.hpp"
#include "Parameters.hpp"

namespace InitConds {

   // component-scope variables
   double y0, dy;
   double x0, dx;

   // =========================================================================
   // Set up

   void setup () {

      // ----------------------------------------------------------------------
      // Declare variables

      double temp;

      // ----------------------------------------------------------------------
      // Initialize the InitConds component

      x0 = Parameters::get_optional<double>("InitConds.x0", 0.00);
      dx = Parameters::get_optional<double>("InitConds.dx", 0.75);
      y0 = Parameters::get_optional<double>("InitConds.y0", 10.0);
      dy = Parameters::get_optional<double>("InitConds.dy", 1.25);

      // ----------------------------------------------------------------------
      // Set the initial data

      if (Driver::restart_dir == "") {
         for (int i = Grid::ilo; i < Grid::ihi; i++) {
            temp = (Grid::x[i] - x0) / dx;
            temp = temp * temp;
            Grid::data[i] = y0 + dy * exp(-temp);
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
