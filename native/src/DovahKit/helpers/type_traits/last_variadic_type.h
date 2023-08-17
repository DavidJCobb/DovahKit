#pragma once
#include <type_traits>

namespace cobb {
   template<typename... Types>
   struct last_variadic_type {
      using type = typename decltype((std::type_identity_t<Types>{}, ...))::type;
   };

   template<typename... Types>
   using last_variadic_type_t = typename last_variadic_type<Types...>::type;
}