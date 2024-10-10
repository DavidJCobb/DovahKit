#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include "./utils/cell_grid_position.h"

namespace dovah {
   class form_stub;

   struct form_stub_addenda {
      std::optional<cell_grid_position> grid_position; // WRLD/CELL/XCLC
      struct {
         //
         // This handles the order of INFOs within a DIAL. We need to know the current 
         // order (i.e. with respect to the active file) as well as the overridden order 
         // (i.e. with respect to all of the active file's masters), so we can compare 
         // them at save time and know when to serialize INFO/PNAM.
         //
         std::vector<form_stub*> dependencies;
         std::vector<form_stub*> active_file;
      } ordered_children;
      form_stub* persistent_cell     = nullptr; // WRLD persistent cell
      form_stub* canonical_landscape = nullptr; // last loaded LAND for a CELL

      void clone_from(const form_stub_addenda&); // shallow copy, and should only copy data that we'd want to copy when, say, duplicating a form

      void sever_references_to_deleted_form(form_stub&, bool just_being_flagged);
   };
}
