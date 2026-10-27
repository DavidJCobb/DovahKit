#pragma once
#include <type_traits>
#include "./elements_usable_as_callable_nttps.h"
#include "../type_traits/templated_call_operator.h"

namespace lu::tuples {
   template<const auto& TupleOrTupleLike, typename Lambda, typename... Args>
   using elements_usable_as_lambda_nttps = elements_usable_as_callable_nttps<
      TupleOrTupleLike,
      templated_call_operator<Lambda>::template nttp_type,
      std::add_lvalue_reference_t<Lambda>,
      Args...
   >;
}