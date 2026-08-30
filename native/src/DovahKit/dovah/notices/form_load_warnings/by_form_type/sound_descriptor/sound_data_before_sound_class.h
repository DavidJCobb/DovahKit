#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::sound_descriptor {
   class sound_data_before_sound_class : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr sound_data_before_sound_class(
            form_stub& stub,
            uint32_t   signature
         )
         :
            base_form_load_warning(stub),
            unexpected_signature(signature)
         {}

         uint32_t unexpected_signature;
   };
}
#include "../../../_util.undef.h"