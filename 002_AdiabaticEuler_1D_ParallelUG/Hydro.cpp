#include "Defines.hpp"

// STL includes
#include<cmath>

// Boost includes

// Includes specific to this code
#include "Driver.hpp"
#include "Grid.hpp"
#include "GridVars.hpp"
#include "Hydro.hpp"
#include "Parameters.hpp"

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
      v_adv = Parameters::get_optional<double>("Hydro.v_adv", 1.0);

      // Maximum allowed fraction of a CFL time step
      f_cfl = Parameters::get_optional<double>("Hydro.f_cfl", 0.75);

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

      Grid::FaceVar fluxes;

      // ----------------------------------------------------------------------
      // Hydro step

      // Compute the fluxes
      compute_fluxes(fluxes);

      // Update the variables
      update(fluxes);

   }

   // =========================================================================
   // Compute the fluxes

   void compute_fluxes (Grid::FaceVar &fluxes) {

      // ----------------------------------------------------------------------
      // Declare variables

      Grid::FaceVar upper, lower;

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

   void reconstruction(Grid::FaceVar &lower, Grid::FaceVar &upper) {

      // ----------------------------------------------------------------------
      // Declare variables

      int ilo = Grid::ilo;
      int ihi = Grid::ihi - 1;

      // ----------------------------------------------------------------------
      // Reconstruct

      lower.init();
      upper.init();
      for (int i = ilo; i < ihi; i++) {
         lower[i] = Grid::data[i];
         upper[i] = Grid::data[i+1];
      }

   }

   // =========================================================================
   // Solve the Riemann problem at each cell interface

   void riemann (Grid::FaceVar &lower, Grid::FaceVar &upper,
         Grid::FaceVar &fluxes) {

      // ----------------------------------------------------------------------
      // Declare variables

      int ilo = Grid::ilo;
      int ihi = Grid::ihi - 1;

      // ----------------------------------------------------------------------
      // Solve the Riemann problem

      fluxes.init();

      if (v_adv == 0) {
         for (int i = ilo; i < ihi; i++) {
            fluxes[i] = 0;
         }
      } else if (v_adv > 0) {
         for (int i = ilo; i < ihi; i++) {
            fluxes[i] = v_adv * lower[i];
         }
      } else {
         for (int i = ilo; i < ihi; i++) {
            fluxes[i] = v_adv * upper[i];
         }
      }

   }

   // =========================================================================
   // Update the cell values based on the fluxes

   void update (Grid::FaceVar &fluxes) {

      // ----------------------------------------------------------------------
      // Declare variables

      double dt = Driver::dt;
      double dx = Grid::dx;
      double dQ;
      int ilo = Grid::ilo;
      int ihi = Grid::ihi - 1;

      // ----------------------------------------------------------------------
      // Update

      for (int i = ilo; i < ihi; i++) {
         dQ = fluxes[i] * dt / dx;
         // Matter flowing out to the right
         Grid::data[i] -= dQ;
         // Matter flowing in from the left
         Grid::data[i+1] += dQ;
      }

   }

}
