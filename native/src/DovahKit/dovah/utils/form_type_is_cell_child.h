#pragma once
#include "../form_types.h"

namespace dovah {
   constexpr bool form_type_is_cell_child(form_type ft) noexcept {
      if (form_type_is_reference(ft))
         return true;
      switch (ft) {
         case form_type::land:
         case form_type::navmesh:
            return true;
      }
      return false;
   }
}
