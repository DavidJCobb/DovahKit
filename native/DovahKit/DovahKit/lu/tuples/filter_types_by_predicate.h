#pragma once
#include "../type_traits/is_std_tuple.h"
#include "./index_sequence_of_types_matching_predicate.h"
#include "./pick_types_by_index_sequence.h"

namespace lu::tuples {
   template<typename Tuple, auto Predicate> requires is_std_tuple_v<Tuple>
   using filter_types_by_predicate = pick_types_by_index_sequence<Tuple, index_sequence_of_types_matching_predicate<Tuple, Predicate>>;
}