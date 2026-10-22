#pragma once
#include "dovah/form_type.h"

namespace dovah::utils::form_types {
   constexpr bool is_ref(form_type ft) {
      switch (ft) {
         case form_type::actor:
         case form_type::reference:
         case form_type::arrow:
         case form_type::barrier:
         case form_type::beam:
         case form_type::cone:
         case form_type::flame:
         case form_type::grenade:
         case form_type::placed_hazard:
         case form_type::missile:
            return true;
      }
      return false;
   }
}
