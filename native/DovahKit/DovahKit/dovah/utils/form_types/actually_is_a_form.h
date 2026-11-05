#pragma once
#include "dovah/form_type.h"

namespace dovah::utils::form_types {
   constexpr bool actually_is_a_form(form_type ft) {
      switch (ft) {
         case form_type::none:
         case form_type::file_header:
         case form_type::file_record_group:
         case form_type::setting:
         case form_type::toft:
         case form_type::unk87:
         case form_type::alias:
         case form_type::reference_alias:
         case form_type::location_alias:
         case form_type::active_magic_effect:
            return false;
      }
      return true;
   }
}