#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::extra_data {
   class water_data_swallowed_subrecord : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr water_data_swallowed_subrecord(
            form_stub& subject,
            uint32_t signature_seen
         )
         :
            base_form_load_warning(subject),
            signature_seen(signature_seen)
         {}
         
         uint32_t signature_seen = 0;
   };
}
#include "../../../_util.undef.h"