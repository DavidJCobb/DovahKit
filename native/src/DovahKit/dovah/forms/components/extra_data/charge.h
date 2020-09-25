#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class charge : public float_extra_data<'XCHG', extra_data_type::charge> {
   };
}