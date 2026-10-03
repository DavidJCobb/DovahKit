#pragma once
#include <cstdint>

namespace dovah::tes {
   enum class group_type : uint32_t {
      forms_of_type            = 0,
      world_children           = 1,
      interior_cell_block      = 2,
      interior_cell_sub_block  = 3,
      exterior_cell_block      = 4,
      exterior_cell_sub_block  = 5,
      cell_children            = 6,
      topic_children           = 7, // DIAL -> INFO
      cell_persistent_children = 8,
      cell_temporary_children  = 9,
   };
}