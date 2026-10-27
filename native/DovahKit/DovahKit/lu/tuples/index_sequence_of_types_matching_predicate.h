#pragma once
#include <type_traits>
#include "./indices_of_types_matching_predicate.h"

namespace lu::tuples {
   namespace impl {
      template<typename TupleOrTupleLike, auto Predicate>
      struct _index_sequence_of_types_matching_predicate {
         static constexpr const auto   indices = indices_of_types_matching_predicate<TupleOrTupleLike, Predicate>;
         static constexpr const size_t count   = indices.size();

         template<typename IS = std::make_index_sequence<count>>
         struct worker;
         template<size_t... I>
         struct worker<std::index_sequence<I...>> {
            using type = std::index_sequence<indices[I]...>;
         };

         using type = typename worker<std::make_index_sequence<count>>::type;
      };
   }

   template<typename TupleOrTupleLike, auto Predicate>
   using index_sequence_of_types_matching_predicate = typename impl::_index_sequence_of_types_matching_predicate<TupleOrTupleLike, Predicate>::type;
}