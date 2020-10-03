#pragma once
#include <functional>

namespace dovah {
   class form_stub;
   //
   namespace form_stub_helpers {
      extern void for_each_child_form(const form_stub* parent, std::function<bool(form_stub*)> functor); // return (true) to stop looping early
      extern form_stub* get_base_form(const form_stub* ref);
      extern form_stub* get_worldspace_persistent_cell(const form_stub*);
      extern form_stub* get_worldspace_cell_by_grid(const form_stub* world, int32_t x, int32_t y);
   }
}