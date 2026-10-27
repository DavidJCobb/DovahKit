#pragma once
#include <type_traits>
#include <tuple>
#include "../type_traits/is_std_tuple.h"

namespace lu::tuples {
   namespace impl {
      template<typename Tuple, typename Indices>
      struct _pick_types_by_index_sequence;
      template<typename Tuple, auto... I>
      struct _pick_types_by_index_sequence<Tuple, std::index_sequence<I...>> {
         using type = std::tuple<std::tuple_element_t<I, Tuple>...>;
      };
   }
   
   template<typename Tuple, typename IS> requires is_std_tuple_v<Tuple>
   using pick_types_by_index_sequence = typename impl::_pick_types_by_index_sequence<Tuple, IS>::type;
}