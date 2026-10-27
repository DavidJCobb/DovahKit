#pragma once
#include <type_traits>
#include "./elements_usable_as_callable_template_params.h"
#include "../type_traits/templated_call_operator.h"

namespace lu::tuples {
   template<typename TupleOrTupleLike, typename Lambda, typename... Args>
   using elements_usable_as_lambda_template_params = elements_usable_as_callable_template_params<
      TupleOrTupleLike,
      templated_call_operator<Lambda>::template type,
      std::add_lvalue_reference_t<Lambda>,
      Args...
   >;
}