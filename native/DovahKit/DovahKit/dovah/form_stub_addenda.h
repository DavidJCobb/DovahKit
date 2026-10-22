#pragma once
#include <optional>
#include "./form_stub_addendum_types/ordered_child_collection.h"
#include "./types/cells/grid_position.h"
namespace dovah {
   class form_stub;
}

namespace dovah {
   class form_stub_addenda {
      public:
         form_stub* persistent_cell     = nullptr; // WRLD persistent cell
         form_stub* canonical_landscape = nullptr; // last loaded LAND for a CELL
         form_stub_addendum_types::ordered_child_collection ordered_children;
         std::optional<types::cells::grid_position> grid_position; // WRLD/CELL/XCLC
   };
}
