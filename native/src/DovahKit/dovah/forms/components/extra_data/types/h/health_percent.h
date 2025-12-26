#pragma once
#include "../common_fundamental.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class health_percent : public common_fundamental<health_percent, 'XHLP', float, 1.0F> {
   };
}