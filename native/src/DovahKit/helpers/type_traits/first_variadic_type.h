#pragma once

namespace cobb {
   template<typename... Types>
   struct first_variadic_type;

   template<typename First, typename... Remaining>
   struct first_variadic_type<First, Remaining...> {
      using type = First;
   };

   template<typename... Types>
   using first_variadic_type_t = typename first_variadic_type<Types...>::type;
}
