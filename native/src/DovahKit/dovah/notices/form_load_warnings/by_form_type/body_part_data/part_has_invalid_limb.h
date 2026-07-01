#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::body_part_data {
   class part_has_invalid_limb : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr part_has_invalid_limb(
            form_stub& stub,
            size_t  which_part,
            uint8_t limb
         )
         :
            base_form_load_warning(stub),
            which_part(which_part),
            limb(limb)
         {}

         size_t  which_part; // relative to all parts, including those discarded during load
         uint8_t limb;
   };
}
#include "../../../_util.undef.h"