#pragma once
#include <cstdint>

namespace dovah {
   class form_stub;
}

namespace dovah::form_stub_helpers {
   extern form_stub* get_worldspace_cell_by_grid(const form_stub* world, int32_t x, int32_t y);
}