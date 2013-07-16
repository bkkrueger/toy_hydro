#ifndef HYDRO_HPP
#define HYDRO_HPP

#include "Defines.hpp"

// STL includes

// Boost includes

// Includes specific to this code
#include "Array.hpp"

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

   void compute_fluxes (Array &fluxes);

   void reconstruction(Array &lower, Array &upper);

   void riemann (Array &lower, Array &upper, Array &fluxes);

   void update (Array &fluxes);

}

#endif
