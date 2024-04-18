#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::package_event_dialogue {
   //
   // Unrecognized subrecord inside of the object. The loader for this object blindly consumes 
   // subrecords until it finds an expected end; if the end is missing, then this will end badly!
   //
   class unrecognized_subrecord : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unrecognized_subrecord(form_stub& subject, uint32_t sig) : base_form_load_warning(subject), signature(sig) {}

         uint32_t signature = 0;
   };
}
#include "../../../_util.undef.h"