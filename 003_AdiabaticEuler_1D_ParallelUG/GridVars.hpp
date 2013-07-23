#ifndef GRIDVARS_HPP
#define GRIDVARS_HPP

#include "Defines.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Grid.hpp"

namespace Grid {

   extern DelayedConst<unsigned int> Ng, Nx_local;
   extern DelayedConst<int> ilo, ihi;
   extern DelayedConst<unsigned int> n_vars;

   // =========================================================================
   // Cell-centered variables
   class CellVar {

      private:

         double *data;
         bool uninitialized;
         unsigned int Nv;

      public:

         CellVar () {
            uninitialized = true;
            data = NULL;
         }

         void init () {
            if (Nx_local.is_set()) {
               if (uninitialized) {
                  Nv = 1;
                  data = new double [Nx_local+2*Ng];
                  uninitialized = false;
               } else {
                  if (Nv != 1) {
                     delete [] data;
                     Nv = 1;
                     data = new double [Nx_local+2*Ng];
                  }
               }
            }
         }

         void init (unsigned int num_vars) {
            if (Nx_local.is_set()) {
               if (uninitialized) {
                  Nv = num_vars;
                  data = new double [(Nx_local+2*Ng)*Nv];
                  uninitialized = false;
               } else {
                  if (num_vars != Nv) {
                     delete [] data;
                     Nv = num_vars;
                     data = new double [(Nx_local+2*Ng)*Nv];
                  }
               }
            }
         }

         ~CellVar() {
            if (!uninitialized) {
               delete [] data;
               data = NULL;
               Nv = 0;
               uninitialized = true;
            }
         }

         double& operator() (int idx) {
            assert(!uninitialized);
            assert(Nv == 1);
            if ((idx < ilo) || (ihi <= idx)) {
               std::stringstream ss;
               ss << "out of range index in CellVar";
               throw std::out_of_range(ss.str());
            } else {
               return data[idx-ilo];
            }
         }

         double& operator() (int idx, unsigned int var) {
            assert(!uninitialized);
            if ((idx < ilo) || (ihi <= idx)) {
               std::stringstream ss;
               ss << "out of range index in CellVar";
               throw std::out_of_range(ss.str());
            } else if (var >= Nv) {
               std::stringstream ss;
               ss << "out of range variable in CellVar";
               throw std::out_of_range(ss.str());
            } else {
               return data[(idx-ilo)*Nv + var];
            }
         }

         unsigned int const var_count () {
            return Nv;
         }

         bool const is_initialized () {
            return !uninitialized;
         }

   };

   // =========================================================================
   // Face-centered variables
   class FaceVar {

      private:

         double *data;
         bool uninitialized;
         unsigned int Nv;

      public:

         FaceVar () {
            uninitialized = true;
            data = NULL;
         }

         void init () {
            if (Nx_local.is_set()) {
               if (uninitialized) {
                  Nv = 1;
                  data = new double [Nx_local+2*Ng-1];
                  uninitialized = false;
               } else {
                  if (Nv != 1) {
                     delete [] data;
                     Nv = 1;
                     data = new double [Nx_local+2*Ng-1];
                  }
               }
            }
         }

         void init (unsigned int num_vars) {
            if (Nx_local.is_set()) {
               if (uninitialized) {
                  Nv = num_vars;
                  data = new double [(Nx_local+2*Ng-1)*Nv];
                  uninitialized = false;
               } else {
                  if (num_vars != Nv) {
                     delete [] data;
                     Nv = num_vars;
                     data = new double [(Nx_local+2*Ng-1)*Nv];
                  }
               }
            }
         }

         ~FaceVar() {
            if (!uninitialized) {
               delete [] data;
               data = NULL;
               Nv = 0;
               uninitialized = true;
            }
         }

         double& operator() (int idx) {
            assert(!uninitialized);
            assert(Nv == 1);
            if ((idx < ilo) || (ihi-1 <= idx)) {
               std::stringstream ss;
               ss << "out of range index in CellVar";
               throw std::out_of_range(ss.str());
            } else {
               return data[idx-ilo];
            }
         }

         double& operator() (int idx, unsigned int var) {
            assert(!uninitialized);
            if ((idx < ilo) || (ihi-1 <= idx)) {
               std::stringstream ss;
               ss << "out of range index in FaceVar";
               throw std::out_of_range(ss.str());
            } else if (Nv <= var) {
               std::stringstream ss;
               ss << "out of range variable in FaceVar";
               throw std::out_of_range(ss.str());
            } else {
               return data[(idx-ilo)*Nv + var];
            }
         }

         unsigned int const var_count () {
            return Nv;
         }

         bool const is_initialized () {
            return !uninitialized;
         }

   };

}

#endif // ifndef GRIDVARS_HPP
