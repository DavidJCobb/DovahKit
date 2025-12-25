#pragma once
#include <cstdint>
namespace dovah::loaded_forms::components::extra_data_types {
   class extra_data;
}

namespace dovah::loaded_forms::components::extra_data_factories {
   extern extra_data_types::extra_data* construct_for_subrecord(uint32_t signature);
}