#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class time_left : public float_extra_data<'XTIM', extra_data_type::time_left> {
   };
}