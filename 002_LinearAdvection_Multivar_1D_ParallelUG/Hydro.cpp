#include "Defines.hpp"

// STL includes
#include<cmath>

// Boost includes

// Includes specific to this code
#include "Driver.hpp"
#include "Grid.hpp"
#include "GridVars.hpp"
#include "Hydro.hpp"
#include "Log.hpp"
#include "Parameters.hpp"

namespace Hydro {

   // component-scope variables
   double v_adv;     // The advection speed
   double f_cfl;     // The maximum allowed fraction of CFL time step

   // Variable indices
   //DelayedConst<unsigned int> idx_fld1, idx_fld2;

   // =========================================================================
   // Variable request list

   void add_variables () {
   }

   // =========================================================================
   // Set up

   void setup () {

      // ----------------------------------------------------------------------
      // Declare variables

      // ----------------------------------------------------------------------
      // Initialize the Hydro component

      Log::write_single(std::string(79,'_') + "\n");
      Log::write_single("Hydro Setup:\n\n");

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
      // Reconstruct

      lower.init(Grid::n_vars);
      upper.init(Grid::n_vars);
      for (int i = Grid::ilo; i < Grid::ihi-1; i++) {
         for (unsigned int v = 0; v < Grid::n_vars; v++) {
            lower(i,v) = Grid::data(i,  v);
            upper(i,v) = Grid::data(i+1,v);
         }
      }

   }

   // =========================================================================
   // Solve the Riemann problem at each cell interface

   void riemann (Grid::FaceVar &lower, Grid::FaceVar &upper,
         Grid::FaceVar &fluxes) {

      // ----------------------------------------------------------------------
      // Solve the Riemann problem

      fluxes.init(Grid::n_vars);

      if (v_adv == 0) {
         for (int i = Grid::ilo; i < Grid::ihi-1; i++) {
            for (unsigned int v = 0; v < Grid::n_vars; v++) {
               fluxes(i,v) = 0;
            }
         }
      } else if (v_adv > 0) {
         for (int i = Grid::ilo; i < Grid::ihi-1; i++) {
            for (unsigned int v = 0; v < Grid::n_vars; v++) {
               fluxes(i,v) = v_adv * lower(i,v);
            }
         }
      } else /*(v_adv < 0)*/ {
         for (int i = Grid::ilo; i < Grid::ihi-1; i++) {
            for (unsigned int v = 0; v < Grid::n_vars; v++) {
               fluxes(i,v) = v_adv * upper(i,v);
            }
         }
      }

   }

   // =========================================================================
   // Update the cell values based on the fluxes

   void update (Grid::FaceVar &fluxes) {

      // ----------------------------------------------------------------------
      // Declare variables

      double dt_dx;
      double dQ;

      // ----------------------------------------------------------------------
      // Update

      dt_dx = Driver::dt / Grid::dx;
      for (int i = Grid::ilo; i < Grid::ihi-1; i++) {
         for (unsigned int v = 0; v < Grid::n_vars; v++) {
            dQ = fluxes(i,v) * dt_dx;
            // Matter flowing out to the right
            Grid::data(i,v) -= dQ;
            // Matter flowing in from the left
            Grid::data(i+1,v) += dQ;
         }
      }

      }

}
