#pragma once
#include <variant>
#include "../../../data/conditions/comparison_operator.h"

namespace dovah {
   namespace loaded_forms::components::conditions {
      using comparison_operator = dovah::conditions::comparison_operator;
   }
   class form_stub;
}

namespace dovah::loaded_forms::components::conditions {
   struct working_comparison {
      constexpr bool operator==(const working_comparison&) const noexcept = default;

      comparison_operator op = comparison_operator::equal;
      std::variant<
         float,
         form_stub* // GLOB
      > operand;
   };
}