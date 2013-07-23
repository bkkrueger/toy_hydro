#ifndef HYDRO_HPP
#define HYDRO_HPP

#include "Defines.hpp"

// STL includes

// Boost includes

// Includes specific to this code
#include "GridVars.hpp"

namespace Hydro {

   // component-scope variables
   extern double v_adv;     // The advection speed
   const int min_guard = 1;

   // Variable indices
   extern DelayedConst<unsigned int> idx_fld1, idx_fld2;

   // =========================================================================
   // Variable request list

   void add_variables ();

   // =========================================================================
   // Set up

   void setup ();

   // =========================================================================
   // Clean up

   void cleanup ();

   // =========================================================================
   // Compute the time step

   double compute_time_step();

   // =========================================================================
   // A single hydrodynamics step

   void one_step ();

   void compute_fluxes (Grid::FaceVar &fluxes);

   void reconstruction(Grid::FaceVar &lower, Grid::FaceVar &upper);

   void riemann (Grid::FaceVar &lower, Grid::FaceVar &upper,
         Grid::FaceVar &fluxes);

   void update (Grid::FaceVar &fluxes);

}

#endif
