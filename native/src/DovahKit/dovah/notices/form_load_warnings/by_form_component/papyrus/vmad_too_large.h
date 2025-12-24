#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::papyrus {
   //
   // Per CK warning messages, VMAD subrecords are not loaded in-game 
   // if they exceed 1MB.
   //
   class vmad_too_large : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr vmad_too_large(form_stub& subject, size_t size) : base_form_load_warning(subject), size(size) {}

         size_t size = 0;
   };
}
#include "../../../_util.undef.h"