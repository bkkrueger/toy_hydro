#ifndef ARRAY_HPP
#define ARRAY_HPP

#include "Defines.hpp"

#include <sstream>
#include <stdexcept>

class Array {

   private:

      double *data;
      unsigned int N;
      int offset;

   public:

      Array() {
         N = 0;
         data = NULL;
         offset = 0;
      }

      Array(unsigned int num_elem) {
         if (num_elem == 0) {
            N = 0;
            data = NULL;
            offset = 0;
         } else {
            N = num_elem;
            data = new double[N];
            offset = 0;
            for (unsigned int i = 0; i < N; i++) {
               data[i] = 0.0;
            }
         }
      }

      Array (int idx_beg, int idx_end) {
         // For consistency with the single-uint constructor, we say that the
         // Array starts at idx_beg but ends one below idx_end (like in
         // Python); this is because the single-uint constructor can be thought
         // of as like Array(N) <=> Array(0,N), which implies that we go up to
         // but do not include idx_end.
         if (idx_end <= idx_beg) {
            throw std::length_error(
                  "upper index must be greater than lower index in Array");
         } else {
            N = idx_end - idx_beg;
            data = new double[N];
            offset = idx_beg;
            for (unsigned int i = 0; i < N; i++) {
               data[i] = 0.0;
            }
         }
      }

      ~Array() {
         delete [] data;
         N = 0;
         offset = 0;
      }

      double& operator[] (int idx) {
         int i = idx - offset;
         if (i < 0) {
            std::stringstream ss;
            ss << "out of range in Array : " << idx << " < " << offset;
            throw std::out_of_range(ss.str());
         } else if (N <= i) {
            std::stringstream ss;
            ss << "out of range in Array : " << idx << " > " << offset+N-1;
            throw std::out_of_range(ss.str());
         } else {
            return data[i];
         }
      }

      unsigned int size() {
         return N;
      }

      int lower_limit() {
         return offset;
      }

      int upper_limit() {
         return offset + N;
      }

      void resize(unsigned int num_elem) {
         if (num_elem == 0) {
            if (N != 0) {
               N = 0;
               delete [] data;
               data = NULL;
               offset = 0;
            }
         } else {
            if (N != 0) {
               delete [] data;
            }
            N = num_elem;
            data = new double[N];
            offset = 0;
            for (unsigned int i = 0; i < N; i++) {
               data[i] = 0.0;
            }
         }
      }

      void resize(int idx_beg, int idx_end) {
         if (idx_end <= idx_beg) {
            throw std::length_error(
                  "upper index must be greater than lower index in Array");
         } else {
            if (N != 0) {
               delete [] data;
            }
            N = idx_end - idx_beg;
            data = new double[N];
            offset = idx_beg;
            for (unsigned int i = 0; i < N; i++) {
               data[i] = 0.0;
            }
         }
      }

};

#endif
