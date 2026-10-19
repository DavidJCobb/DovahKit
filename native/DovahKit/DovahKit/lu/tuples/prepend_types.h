#pragma once
#include <tuple>
#include "../type_traits/is_std_tuple.h"

namespace lu::tuples {
   namespace impl {
      template<typename, typename...>
      struct _prepend_types;

      template<typename... TypesToPrepend, typename... TypesAlreadyPresent>
      struct _prepend_types<std::tuple<TypesAlreadyPresent...>, TypesToPrepend...> {
         using type = std::tuple<TypesToPrepend..., TypesAlreadyPresent...>;
      };
   }

   template<typename Tuple, typename... TypesToPrepend>
      requires is_std_tuple_v<Tuple>
   using prepend_types = typename impl::_prepend_types<Tuple, TypesToPrepend...>::type;
}