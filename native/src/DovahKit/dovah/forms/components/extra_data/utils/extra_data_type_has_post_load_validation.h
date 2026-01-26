#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra_data_utils {
   extern bool extra_data_type_has_post_load_validation(extra_data_types::extra_data::typecode_type);
   extern bool extra_data_has_post_load_validation(const extra_data_types::extra_data&);
}