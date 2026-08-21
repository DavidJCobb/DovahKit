#pragma once
#include <variant>
#include "dovah/data/conditions/comparison_operator.h"
#include "dovah/form_reference_t.h"

namespace dovah::loaded_forms::components::conditions {
   using comparison_operator = dovah::conditions::comparison_operator;

   struct comparison_data {
      comparison_operator op = comparison_operator::equal;
      std::variant<
         float,
         form_reference_t // GLOB
      > operand;
   };
}