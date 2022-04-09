#pragma once
#include <array>
#include <type_traits>
#include "concepts.h"
#include "type_traits/is_std_array.h"

namespace cobb {
   namespace impl::array_concat {
      template<typename... Types> struct total_size {
         static constexpr size_t value = (std::tuple_size_v<Types> +...);
      };
      template<typename... Types> struct value_type {
         template<typename T, typename... Types> struct first {
            using type = T::value_type;
         };

         using type = first<Types...>::type;
      };
   }

   template<typename... Types> requires requires(Types... args) {
      requires (is_std_array<Types> && ...);
      requires (std::is_same_v<typename impl::array_concat::value_type<Types...>::type, typename Types::value_type> && ...);
   }
   constexpr std::array<
      typename impl::array_concat::value_type<Types...>::type,
      impl::array_concat::total_size<Types...>::value
   > array_concat(Types... arrays) {
      using value_type = typename impl::array_concat::value_type<Types...>::type;
      constexpr size_t size = impl::array_concat::total_size<Types...>::value;
      //
      std::array<value_type, size> out = {};
      size_t i = 0;
      (
         (
            std::copy(arrays.cbegin(), arrays.cend(), out.begin() + i),
            i += arrays.size()
         ),
         ...
      );
      return out;
   }
}