#include "./classify_boolean_comparison.h"
#include <utility> // std::unreachable
#include "dovah/forms/components/conditions.h"

namespace dovah::utils::conditions {
   extern boolean_test_classification classify_boolean_comparison(const loaded_forms::components::condition& cnd) {
      const auto& cmp = cnd.get_comparison();
      if (!std::holds_alternative<float>(cmp.operand))
         return boolean_test_classification::not_a_boolean_check;
      auto operand = std::get<float>(cmp.operand);
      switch (cmp.op) {
         using enum loaded_forms::components::conditions::comparison_operator;
         using enum boolean_test_classification;
         case equal:
            if (operand == 0.0F)
               return inverted;
            if (operand == 1.0F)
               return normal;
            return not_a_boolean_check;
         case not_equal:
            if (operand == 1.0F)
               return inverted;
            if (operand == 0.0F)
               return normal;
            return not_a_boolean_check;
         case less_or_equal:
            if (operand <  0.0F)
               return always_false;
            if (operand >= 1.0F)
               return always_true;
            return inverted;
         case less:
            if (operand <= 0.0F)
               return always_false;
            if (operand > 1.0F)
               return always_true;
            return inverted;
         case greater_or_equal:
            if (operand <= 0.0F)
               return always_true;
            if (operand > 1.0F)
               return always_false;
            return normal;
         case greater:
            if (operand < 0.0F)
               return always_true;
            if (operand >= 1.0F)
               return always_false;
            return normal;
      }
      std::unreachable();
      return boolean_test_classification::not_a_boolean_check;
   }
}