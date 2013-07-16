#ifndef HYDRO_HPP
#define HYDRO_HPP

// TODO : Do I want to turn this into a class to allow public/private?  There
// are also unnamed namespaces, but I don't know the usage of those so I'm not
// yet confident enough to use them myself.

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
