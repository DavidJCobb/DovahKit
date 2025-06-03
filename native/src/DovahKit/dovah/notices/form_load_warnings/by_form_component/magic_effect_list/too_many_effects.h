#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "dovah/forms/components/magic_effect_list.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::magic_effect_list {
   //
   // Some form types limit the number of effects they can have.
   //
   class too_many_effects : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr too_many_effects(
            form_stub& subject,
            size_t effects_contained,
            size_t effects_allowed
         )
         :
            base_form_load_warning(subject),
            effects_contained(effects_contained),
            effects_allowed(effects_allowed)
         {}

         size_t effects_contained = 0;
         size_t effects_allowed   = 0;

         // The game won't even load effects past this point.
         const size_t hard_maximum = loaded_forms::components::magic_effect_list::hard_maximum_count;
   };
}
#include "../../../_util.undef.h"