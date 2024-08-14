#include "./is_worldspace_persistent_cell.h"
#include "../../form_stub.h"
#include "../../form_types.h"
#include "./get_worldspace_persistent_cell.h"

namespace dovah::form_stub_helpers {
   extern bool is_worldspace_persistent_cell(const form_stub& cell) {
      auto* parent = cell.get_parent_form();
      if (!parent)
         return false;
      if (parent->form_type != form_type::worldspace)
         return false;
      return &cell == get_worldspace_persistent_cell(parent);
   }
}