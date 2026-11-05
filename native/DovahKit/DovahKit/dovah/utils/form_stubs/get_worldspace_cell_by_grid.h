#pragma once
#include <cstdint>
namespace dovah {
   class form_stub;
}

namespace dovah::utils::form_stubs {
   extern form_stub* get_worldspace_cell_by_grid(const form_stub& world, int32_t gx, int32_t gy);
}