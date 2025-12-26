#pragma once
#include "../common_fundamental.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class rank : public common_fundamental<rank, 'XRNK', int32_t, -1> {
      public:
         static constexpr const int32_t sentinel_value_for_unset = -1;
   };
}