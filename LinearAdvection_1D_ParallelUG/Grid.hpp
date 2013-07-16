#ifndef GRID_HPP
#define GRID_HPP

#include "Defines.hpp"

// STL includes

// Boost includes

// Includes specific to this code
#include "Array.hpp"

namespace Grid {

   // component-scope variables
   extern unsigned int Ng;     // the number of guard cells around the borders

   extern unsigned int Nx_global;
   extern unsigned int Nx_local;
   extern double xmin, xmax;   // limits in the x direction
   extern double dx;           // grid spacing in the x direction

   extern Array x;             // array of x coordinates
   extern Array data;          // the data grid
   extern int ilo, ihi;        // Arrays include indices ilo to ihi-1

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
