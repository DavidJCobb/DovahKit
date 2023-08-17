#pragma once
#include <tuple>

namespace cobb::tuples {
   namespace impl {
      template<typename Tuple, typename IndexSequence>
      struct _pick_types_by_indices;

      template<typename Tuple, size_t... Indices>
      struct _pick_types_by_indices<Tuple, std::index_sequence<Indices...>> {
         using type = std::tuple<std::tuple_element_t<Indices, Tuple>...>;
      };
   }

   // Inspired by: https://devblogs.microsoft.com/oldnewthing/20200624-00/?p=103902
   // "Mundane std::tuple tricks: Selecting via an index sequence, part 2" by Raymond Chen (June 24th, 2020)
   template<typename Tuple, typename IndexSequence>
   using pick_types_by_indices = impl::_pick_types_by_indices<Tuple, IndexSequence>::type;
}