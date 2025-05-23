#pragma once
#include "../../../base_form_load_warning.h"

#include "dovah/forms/MagicEffect.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::magic_effect {
   class redundant_sound : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         using effect_sound_type = loaded_forms::MagicEffect::effect_sound_type;

      public:
         constexpr redundant_sound(
            form_stub& stub,
            effect_sound_type type,
            form_stub* descriptor
         )
         :
            base_form_load_warning(stub),
            type(type),
            descriptor(descriptor)
         {}

         effect_sound_type type;
         form_stub*        descriptor = nullptr;
   };
}
#include "../../../_util.undef.h"