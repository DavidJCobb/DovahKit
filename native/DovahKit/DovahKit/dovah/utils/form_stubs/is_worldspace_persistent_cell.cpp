#include "./is_worldspace_persistent_cell.h"
#include "./get_parent_form.h"
#include "../../form_stub.h"
#include "../../form_stub_addenda.h"
#include "../../form_type.h"

namespace dovah::utils::form_stubs {
   extern bool is_worldspace_persistent_cell(const form_stub& cell) {
      const auto* parent = get_parent_form(cell);
      if (!parent)
         return false;
      if (parent->form_type != form_type::worldspace)
         return false;
      if (!parent->addenda)
         return false;
      return &cell == parent->addenda->persistent_cell;
   }
}