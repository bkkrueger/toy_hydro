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
