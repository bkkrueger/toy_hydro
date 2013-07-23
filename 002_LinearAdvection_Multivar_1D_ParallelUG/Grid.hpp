#ifndef GRID_HPP
#define GRID_HPP

#include "Defines.hpp"

// STL includes

// Boost includes

// Includes specific to this code
#include "GridVars.hpp"
#include "Support.hpp"

namespace Grid {

   // component-scope variables
   extern DelayedConst<unsigned int> Ng;     // the number of guard cells around the borders

   extern DelayedConst<unsigned int> Nx_global;
   extern DelayedConst<unsigned int> Nx_local;
   extern DelayedConst<double> xmin, xmax;   // limits in the x direction
   extern DelayedConst<double> dx;           // grid spacing in the x direction

   extern CellVar x;             // array of x coordinates
   extern CellVar data;          // the data grid
   extern DelayedConst<int> ilo, ihi;  // arrays include indices ilo to ihi-1

   // The processor IDs of the lower and upper neighbors
   extern DelayedConst<int> neigh_lo, neigh_hi;

   // =========================================================================
   // Add a new variable to the Grid

   unsigned int add_variable(std::string new_var);

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
