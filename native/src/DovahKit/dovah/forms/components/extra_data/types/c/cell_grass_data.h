#pragma once
#include "../common_binary.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class cell_grass_data : public common_binary<cell_grass_data, 'XCGD'> {
   };
}