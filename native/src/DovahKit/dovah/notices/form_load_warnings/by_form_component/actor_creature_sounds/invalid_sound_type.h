#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::actor_creature_sounds {
   class invalid_sound_type : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_sound_type(
            form_stub& subject,
            uint32_t type
         ) :
            base_form_load_warning(subject),
            type(type)
         {}

         uint32_t type = 0;
   };
}
#include "../../../_util.undef.h"