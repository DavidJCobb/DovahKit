#pragma once
#include "../common_buffer.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class occlusion_plane_ref_data : public common_buffer<occlusion_plane_ref_data, 'XORD', 10> {
   };
}