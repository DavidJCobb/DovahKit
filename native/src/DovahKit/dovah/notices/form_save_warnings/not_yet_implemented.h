#pragma once
#include <cstdint>
#include "../base_form_save_warning.h"

#include "../_util.define.h"
namespace dovah::notices::form_save_warnings {
   //
   // Saving this data is not yet implemented.
   //
   class not_yet_implemented : public base_form_save_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr not_yet_implemented(form_stub& subject, uint32_t sig) : base_form_save_warning(subject), subrecord_signature(sig) {}

         uint32_t subrecord_signature = 0;
   };
}
#include "../_util.undef.h"