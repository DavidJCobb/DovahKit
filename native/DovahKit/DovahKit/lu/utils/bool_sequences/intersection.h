#pragma once
#include <utility>

namespace lu::utils::bool_sequences {
   namespace impl {
      template<typename A, typename B>
      struct _intersection;

      template<bool... A, bool... B>
      struct _intersection<std::integer_sequence<bool, A...>, std::integer_sequence<bool, B...>> {
         using type = std::integer_sequence<bool, (A && B)...>;
      };
   }

   // Given two `std::integer_sequence<bool, ...>` types of equivalent length, produces 
   // a new such type whose elements are `true` only if the corresponding elements in 
   // both of the original sequences are `true`.
   template<typename A, typename B>
   using intersection = impl::_intersection<A, B>::type;
}