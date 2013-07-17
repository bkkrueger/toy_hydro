#ifndef SUPPORT_HPP
#define SUPPORT_HPP

#include <cassert>

// ============================================================================
// A pseudo-const variable: It can be assigned once, but subsequently is
// effectively constant.
// Adapted from Stack Overflow question #7880170

template <class T>
class DelayedConst {

   private: 
      T val;         // the value
      bool assigned; // is it assigned?

   public: 

      // Constructor
      DelayedConst() : assigned(false) {
      }

      // Assignment
      DelayedConst& operator= (T v) {
         assert(!assigned); 
         val = v; 
         assigned = true; 
         return *this; 
      }

      // Read the value
      operator T() const { 
         assert(assigned); 
         return val; 
      }

      // Read the value (explicit case to base type)
      T value() const {
         assert(assigned);
         return val;
      }

      // Has the value been set?
      bool is_set() const {
         return assigned;
      }

};

#endif // ifndef SUPPORT_HPP
