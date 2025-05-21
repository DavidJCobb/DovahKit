#pragma once
#include "../../../base_form_load_warning.h"

#include "dovah/forms/SoundDescriptor.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::sound_descriptor {
   //
   // When SNDR is loaded at startup, it expects GNAM to come after CNAM, and will 
   // blindly swallow whatever subrecord comes next. However, when the data is loaded 
   // on-demand during play, it seems to be more sensible.
   //
   class unexpected_subrecord_after_cnam : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unexpected_subrecord_after_cnam(
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