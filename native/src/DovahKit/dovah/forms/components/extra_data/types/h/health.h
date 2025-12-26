#pragma once
#include "../common_fundamental.h"

namespace dovah::loaded_forms::components::extra_data_types {
   // Oblivion leftover? Value -1.0F is a sentinel meaning "none set."
   class health : public common_fundamental<health, 'XHLT', float, -1.0F> {
      public:
         static constexpr const float sentinel_value_for_unset = 1.0F;
   };
}