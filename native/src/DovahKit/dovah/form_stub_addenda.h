#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include "./form_stub_addenda/ordered_child_collection.h"
#include "./utils/cell_grid_position.h"

namespace dovah {
   class form_stub;

   struct form_stub_addenda {
      std::optional<cell_grid_position> grid_position; // WRLD/CELL/XCLC
      form_stub_addendum_types::ordered_child_collection ordered_children;
      form_stub* persistent_cell     = nullptr; // WRLD persistent cell
      form_stub* canonical_landscape = nullptr; // last loaded LAND for a CELL

      void clone_from(const form_stub_addenda&); // shallow copy, and should only copy data that we'd want to copy when, say, duplicating a form

      void sever_references_to_deleted_form(form_stub&, bool just_being_flagged);
   };
}
