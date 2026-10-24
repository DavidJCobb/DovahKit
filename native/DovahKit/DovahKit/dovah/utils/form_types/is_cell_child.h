#pragma once
#include "dovah/form_type.h"
#include "./is_ref.h"

namespace dovah::utils::form_types {
   constexpr bool is_cell_child(form_type ft) {
      if (is_ref(ft))
         return true;
      switch (ft) {
         case form_type::land:
         case form_type::navmesh:
            return true;
      }
      return false;
   }
}
