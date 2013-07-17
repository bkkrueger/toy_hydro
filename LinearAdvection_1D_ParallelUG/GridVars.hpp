#ifndef GRIDVARS_HPP
#define GRIDVARS_HPP

#include "Defines.hpp"

#include <cassert>
#include <sstream>
#include <stdexcept>

namespace Grid {

   // =========================================================================
   // Cell-centered variables
   class CellVar {

      private:

         double *data;
         bool uninitialized;

      public:

         CellVar () {
            uninitialized = true;
            data = NULL;
            init();
         }

         void init () {
            if (uninitialized && Nx_local.is_set()) {
               data = new double [Nx_local+2*Ng];
               uninitialized = false;
            }
         }

         ~CellVar() {
            if (!uninitialized) {
               delete [] data;
               data = NULL;
               uninitialized = true;
            }
         }

         double& operator[] (int idx) {
            assert(!uninitialized);
            if (idx < ilo) {
               std::stringstream ss;
               ss << "out of range in Array : " << idx << " < " << ilo;
               throw std::out_of_range(ss.str());
            } else if (ihi <= i) {
               std::stringstream ss;
               ss << "out of range in Array : " << idx << " > " << ihi-1;
               throw std::out_of_range(ss.str());
            } else {
               return data[idx-ilo];
            }
         }

   // =========================================================================
   // Face-centered variables
   class FaceVar {

      private:

         double *data;
         bool uninitialized;

      public:

         FaceVar () {
            uninitialized = true;
            data = NULL;
            init();
         }

         void init () {
            if (uninitialized && Nx_local.is_set()) {
               data = new double [Nx_local+2*Ng-1];
               uninitialized = false;
            }
         }

         ~FaceVar() {
            if (!uninitialized) {
               delete [] data;
               data = NULL;
               uninitialized = true;
            }
         }

         double& operator[] (int idx) {
            assert(!uninitialized);
            if (idx < ilo) {
               std::stringstream ss;
               ss << "out of range in Array : " << idx << " < " << ilo;
               throw std::out_of_range(ss.str());
            } else if (ihi-1 <= i) {
               std::stringstream ss;
               ss << "out of range in Array : " << idx << " > " << ihi-2;
               throw std::out_of_range(ss.str());
            } else {
               return data[idx-ilo];
            }
         }

   };

}

#endif // ifndef GRIDVARS_HPP
