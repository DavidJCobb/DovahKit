#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class effect_entry_point_has_mismatched_data_for_type : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr effect_entry_point_has_mismatched_data_for_type(
            form_stub& stub,
            size_t which_effect,
            uint32_t subrecord
         )
         :
            base_form_load_warning(stub),
            which_effect(which_effect),
            subrecord(subrecord)
         {}

         size_t which_effect;
         uint32_t subrecord;
   };
}
#include "../../../_util.undef.h"