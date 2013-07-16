// TODO : Do I want to turn this into a class to allow public/private?  There
// are also unnamed namespaces, but I don't know the usage of those so I'm not
// yet confident enough to use them myself.

// STL includes
#include <cmath>

// Boost includes

// Includes specific to this code
#include "Driver.hpp"
#include "Grid.hpp"

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

      x0 = Driver::parameter_with_default<double>("InitConds.x0", 0.00);
      dx = Driver::parameter_with_default<double>("InitConds.dx", 0.75);
      y0 = Driver::parameter_with_default<double>("InitConds.y0", 10.0);
      dy = Driver::parameter_with_default<double>("InitConds.dy", 1.25);

      // ----------------------------------------------------------------------
      // Set the initial data

      if (Driver::restart_dir == "") {
         for (unsigned int i = 0; i < Grid::x.size(); i++) {
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
