#pragma once
#include <cstdint>

namespace dovah::conditions {
   enum class comparison_operator {
      equal            = 0,
      not_equal        = 1,
      greater          = 2,
      greater_or_equal = 3,
      less             = 4,
      less_or_equal    = 5,
   };
}