#pragma once
#include <array>
#include <type_traits>
#include "../concepts.h"

namespace cobb::arrays {

   namespace impl::_make {
      template<typename T> struct is_string_literal {
         static constexpr bool value = false;
      };
      template<size_t N> struct is_string_literal<const char(&)[N]> {
         static constexpr bool value = true;
      };
   }

   //
   // Helper template for creating an array of an arbitrary length but with a specific 
   // desired type. Includes a specialization to ensure that string literals are used 
   // as `const char*` and not `const char(&)[N]`.
   //
   template<typename... Types> requires (cobb::all_same<Types...> || (impl::_make::is_string_literal<Types>::value && ...))
   constexpr std::array<std::tuple_element_t<0, std::tuple<std::decay_t<Types>...>>, sizeof...(Types)> make(Types&&... args) {
      using base_value_type = std::tuple_element_t<0, std::tuple<std::decay_t<Types>...>>;
      using value_type = std::conditional_t<
         (impl::_make::is_string_literal<Types>::value && ...),
         const char*,
         base_value_type
      >;
      //
      std::array<value_type, sizeof...(Types)> out = { args... };
      return out;
   };
}
