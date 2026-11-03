#pragma once
#include "lu/utils/nominal_integer_type.h"
#include "./bare_localized_string_id.h"
#include "./localized_string_type.h"

namespace dovah {
   template<localized_string_type V>
   class localized_string_id : public lu::utils::nominal_integer_type<localized_string_id<V>, bare_localized_string_id> {
      private:
         using _my_base = lu::utils::nominal_integer_type<localized_string_id<V>, bare_localized_string_id>;
      public:
         using integral_type = _my_base::integral_type;
         using _my_base::nominal_integer_type;
   };
}