#pragma once
#include "./get_worldspace_persistent_cell.h"
#include "../../form_stub.h"
#include "../../form_stub_addenda.h"
#include "../../form_types.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_worldspace_persistent_cell(const form_stub& world) {
      if (world.form_type != form_type::worldspace)
         return nullptr;
      if (!world.addenda)
         return nullptr;
      return world.addenda->persistent_cell;
   }
}