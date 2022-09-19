#pragma once
#include <tuple>
#include <type_traits>

namespace cobb::tuples {
   namespace impl {
      template<typename Tuple, typename Desired>
      struct _contains_type;

      template<typename Desired, typename... Types>
      struct _contains_type<std::tuple<Types...>, Desired> {
         static constexpr bool value = (std::is_same_v<Types, Desired> || ...);
      };
   }

   template<typename Tuple, typename T>
   constexpr bool contains_type = impl::_contains_type<std::decay_t<Tuple>, T>::value;
}