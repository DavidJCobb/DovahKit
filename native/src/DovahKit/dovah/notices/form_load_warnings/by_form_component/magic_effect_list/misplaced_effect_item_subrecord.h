#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::magic_effect_list {
   //
   // The game assumes that the subrecord after EFID is EFIT.
   //
   class misplaced_effect_item_subrecord : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr misplaced_effect_item_subrecord(
            form_stub& subject
         )
         :
            base_form_load_warning(subject)
         {}
   };
}
#include "../../../_util.undef.h"