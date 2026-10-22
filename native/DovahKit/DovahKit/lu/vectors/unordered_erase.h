#pragma once
#include <algorithm>

namespace lu::vectors {
   //
   // Quickly erase from an `std::vector` by avoiding having to shuffle all elements 
   // when erasing values from the middle: we just move the to-be-erased value to 
   // the end and then erase it.
   //
   template<typename Vector, typename ValueType = typename Vector::value_type>
   void unordered_erase(Vector& vector, const ValueType& value) {
      auto it = std::find(vector.begin(), vector.end(), value);
      if (it != vector.end()) {
         std::iter_swap(it, vector.end() - 1);
         vector.resize(vector.size() - 1);
      }
   }

   template<typename Vector, typename UnaryPredicate>
   void unordered_erase(Vector& vector, UnaryPredicate&& pred) {
      auto it = std::find_if(vector.begin(), vector.end(), pred);
      if (it != vector.end()) {
         std::iter_swap(it, vector.end() - 1);
         vector.resize(vector.size() - 1);
      }
   }

   template<typename Vector>
   void unordered_erase(Vector& vector, typename Vector::iterator&& it) {
      if (it != vector.end()) {
         std::iter_swap(it, vector.end() - 1);
         vector.resize(vector.size() - 1);
      }
   }
}