#pragma once
#include "../utils/boolean_type_trait_to_lambda.h"
#include "./filter_types_by_predicate.h"

namespace lu::tuples {
   template<typename Tuple, typename Trait>
   using filter_types_by_trait = filter_types_by_predicate<Tuple, utils::boolean_type_trait_to_lambda<Trait>>;
}