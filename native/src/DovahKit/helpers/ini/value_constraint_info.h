#pragma once
#include <limits>
#include <type_traits>
#include "./types.h"

namespace cobb::ini {
   template<typename T> requires value_types::has_key<T>
   struct value_constraint_info {
      constexpr bool allows(const T& v) const noexcept {
         return true;
      }
   };
   
   template<typename T> requires std::is_arithmetic_v<T>
   struct value_constraint_info<T> {
      T min = std::numeric_limits<T>::lowest();
      T max = std::numeric_limits<T>::max();

      constexpr bool allows(T v) const noexcept {
         return (v >= min) && (v <= max);
      }
   };
}