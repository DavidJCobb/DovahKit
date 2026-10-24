#pragma once
#include "dovah/form_stub.h"
#include "dovah/form_type.h"
#include "./get_parent_form.h"

namespace dovah::utils::form_stubs {
   inline bool is_interior_cell(const form_stub& stub) {
      if (stub.form_type != form_type::cell)
         return false;
      return !get_parent_form(stub);
   }
}