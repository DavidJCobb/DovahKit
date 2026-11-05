#pragma once
#include <type_traits>

namespace lu::concepts {
   template<typename T>
   concept supports_range_based_for_loops = (
      // Is a C array?
      (std::is_array_v<T> ?
         // If so, must be a bounded array.
         std::is_unbounded_array_v<T>
      :
         // Else:
         (
            // Is a class with `begin` and `end` member functions?
            (
               std::is_class_v<T> &&
               requires(T v) {
                  { v.begin() };
                  { v.end() };
               }
            ) ||
            // Can find `begin` and `end` free functions via ADL?
            requires (T) {
               { begin(std::type_identity<T>{}) };
               { end(std::type_identity<T>{}) };
            }
         )
      )
   );
}