// TODO : Do I want to turn this into a class to allow public/private?  There
// are also unnamed namespaces, but I don't know the usage of those so I'm not
// yet confident enough to use them myself.

// STL includes
#include<cmath>

// Boost includes

// Includes specific to this code
#include "Array.hpp"
#include "Driver.hpp"
#include "Grid.hpp"
#include "Hydro.hpp"

namespace Hydro {

   // component-scope variables
   double v_adv;     // The advection speed
   double f_cfl;     // The maximum allowed fraction of CFL time step

   // =========================================================================
   // Set up

   void setup () {

      // ----------------------------------------------------------------------
      // Declare variables

      // ----------------------------------------------------------------------
      // Initialize the Hydro component

      // Advection speed
      v_adv = Driver::parameter_with_default<double>("Hydro.v_adv", 1.0);

      // Maximum allowed fraction of a CFL time step
      f_cfl = Driver::parameter_with_default<double>("Hydro.f_cfl", 0.75);

   }

   // =========================================================================
   // Clean up

   void cleanup () {
   }

   // =========================================================================
   // Compute the time step

   double compute_time_step() {
      // dx/dt_CFL = v_adv --> dt_CFL = dx / v_adv --> dt = f_cfl * dt_CFL
      double dt;
      dt = f_cfl * Grid::dx / v_adv;
      return dt;
   }

   // =========================================================================
   // A single hydrodynamics step

   void one_step () {

      // ----------------------------------------------------------------------
      // Declare variables

      Array fluxes;

      // ----------------------------------------------------------------------
      // Hydro step

      // Compute the fluxes
      compute_fluxes(fluxes);

      // Update the variables
      update(fluxes);

   }

   // =========================================================================
   // Compute the fluxes

   void compute_fluxes (Array &fluxes) {

      // ----------------------------------------------------------------------
      // Declare variables

      Array upper, lower;

      // ----------------------------------------------------------------------
      // Compute the fluxes

      // Reconstruction
      reconstruction(lower, upper);

      // Riemann solve
      riemann(lower, upper, fluxes);

   }

   // =========================================================================
   // Reconstruct the lower/upper states for the Riemann problem at each cell
   // interface.

   void reconstruction(Array &lower, Array &upper) {

      // ----------------------------------------------------------------------
      // Declare variables

      unsigned int N = Grid::data.size() - 1;

      // ----------------------------------------------------------------------
      // Reconstruct

      lower.resize(N);
      upper.resize(N);
      for (unsigned int i = 0; i < N; i++) {
         lower[i] = Grid::data[i];
         upper[i] = Grid::data[i+1];
      }

   }

   // =========================================================================
   // Solve the Riemann problem at each cell interface

   void riemann (Array &lower, Array &upper, Array &fluxes) {

      // ----------------------------------------------------------------------
      // Declare variables

      // ----------------------------------------------------------------------
      // Solve the Riemann problem

      fluxes.resize(lower.size());

      if (v_adv == 0) {
         for (unsigned int i = 0; i < fluxes.size(); i++) {
            fluxes[i] = 0;
         }
      } else if (v_adv > 0) {
         for (unsigned int i = 0; i < fluxes.size(); i++) {
            fluxes[i] = v_adv * lower[i];
         }
      } else {
         for (unsigned int i = 0; i < fluxes.size(); i++) {
            fluxes[i] = v_adv * upper[i];
         }
      }

   }

   // =========================================================================
   // Update the cell values based on the fluxes

   void update (Array &fluxes) {

      // ----------------------------------------------------------------------
      // Declare variables

      double dt = Driver::dt;
      double dx = Grid::dx;
      double dQ;

      // ----------------------------------------------------------------------
      // Update

      for (unsigned int i = 0; i < fluxes.size(); i++) {
         dQ = fluxes[i] * dt / dx;
         // Matter flowing out to the right
         Grid::data[i] -= dQ;
         // Matter flowing in from the left
         Grid::data[i+1] += dQ;
      }

   }

}
