#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <stdexcept>

class Array {

   private:

      double *data;
      unsigned int N;

   public:

      Array() {
         N = 0;
         data = NULL;
      }

      Array(unsigned int num_elem) {
         if (num_elem == 0) {
            N = 0;
            data = NULL;
         } else {
            N = num_elem;
            data = new double[N];
         }
      }

      ~Array() {
         delete [] data;
         N = 0;
      }

      double& operator[] (unsigned int idx) {
         if (idx >= N) {
            throw std::out_of_range("out of range");
         } else {
            return data[idx];
         }
      }

      unsigned int size() {
         return N;
      }

      void resize(unsigned int num_elem) {
         if (N != num_elem) {
            if (num_elem == 0) {
               N = 0;
               delete [] data;
               data = NULL;
            } else {
               double *temp = new double[num_elem];
               for (unsigned int i = 0; (i < num_elem) && (i < N); i++) {
                  temp[i] = data[i];
               }
               delete [] data;
               data = temp;
               N = num_elem;
            }
         }
      }

};

#endif
