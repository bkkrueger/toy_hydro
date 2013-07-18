#ifndef GRID_HPP
#define GRID_HPP

// TODO : Do I want to turn this into a class to allow public/private?  There
// are also unnamed namespaces, but I don't know the usage of those so I'm not
// yet confident enough to use them myself.

// STL includes

// Boost includes

// Includes specific to this code
#include "Array.hpp"

namespace Grid {

   // component-scope variables
   extern unsigned int Ng;     // the number of guard cells around the borders

   extern unsigned int Nx;     // number of internal cells in x direction
   extern double xmin, xmax;   // limits in the x direction
   extern double dx;           // grid spacing in the x direction

   extern Array x;             // array of x coordinates
   extern Array data;          // the data grid

   // =========================================================================
   // Set up

   void setup ();

   // =========================================================================
   // Clean up

   void cleanup ();

   // =========================================================================
   // Fill boundary conditions

   void fill_boundary_conditions();

   // =========================================================================
   // Write the data to a file

   std::string write_data ();

   // =========================================================================
   // Load data from a file

   void read_data();

}

#endif
