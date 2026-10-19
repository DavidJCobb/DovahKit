#pragma once
#include <type_traits>
#include <tuple>
#include "../type_aliases/tuple_like_to_tuple_type.h"

namespace lu::tuples {
   namespace impl {
      template<typename TupleOrTupleLike, typename Desired>
      struct _contains_type;

      template<typename Desired, typename... Elements> requires (sizeof...(Elements) > 0)
      struct _contains_type<std::tuple<Elements...>, Desired>
         : public std::bool_constant<(std::is_same_v<Elements, Desired> || ...)>
      {};

      template<typename Desired>
      struct _contains_type<std::tuple<>, Desired> : std::false_type {};
   }

   template<typename TupleOrTupleLike, typename Desired>
   constexpr const bool contains_type = impl::_contains_type<tuple_like_to_tuple_type<TupleOrTupleLike>, Desired>::value;
}
