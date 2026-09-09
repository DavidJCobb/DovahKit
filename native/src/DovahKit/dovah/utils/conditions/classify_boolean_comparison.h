#pragma once
namespace dovah::loaded_forms::components {
   class condition;
}

namespace dovah::utils::conditions {
   enum class boolean_test_classification {
      not_a_boolean_check,
      normal,       // e.g. GetIsID == 1 || GetIsID != 0
      inverted,     // e.g. GetIsID != 1 || GetIsID == 0
      always_false, // e.g. GetIsID >  1 || GetIsID <  0
      always_true,  // e.g. GetIsID <= 1 || GetIsID >= 0
   };

   extern boolean_test_classification classify_boolean_comparison(const loaded_forms::components::condition&);
}