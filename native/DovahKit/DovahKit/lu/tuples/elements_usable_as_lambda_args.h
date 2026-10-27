#pragma once
#include "./elements_usable_as_callable_args.h"

namespace lu::tuples {
   template<typename TupleOrTupleLike, typename Lambda>
   using elements_usable_as_lambda_args = elements_usable_as_callable_args<
      TupleOrTupleLike,
      Lambda
   >;
}