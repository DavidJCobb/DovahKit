#pragma once
#include "../common_fundamental.h"

namespace dovah::loaded_forms::components::extra_data_types {
   // The value is a collision layer ID.
   class collision_data : public common_fundamental<collision_data, 'XTRI', uint32_t> {
   };
}