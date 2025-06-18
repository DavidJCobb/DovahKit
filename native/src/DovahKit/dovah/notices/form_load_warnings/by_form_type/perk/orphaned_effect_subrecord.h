#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class orphaned_effect_subrecord : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr orphaned_effect_subrecord(
            form_stub& stub,
            uint32_t subrecord
         )
         :
            base_form_load_warning(stub),
            subrecord(subrecord)
         {}

         uint32_t subrecord;
   };
}
#include "../../../_util.undef.h"