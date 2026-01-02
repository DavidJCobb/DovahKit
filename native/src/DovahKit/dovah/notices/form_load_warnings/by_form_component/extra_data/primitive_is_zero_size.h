#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::extra_data {
   class primitive_is_zero_size : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr primitive_is_zero_size(
            form_stub& subject
         )
         :
            base_form_load_warning(subject)
         {}
   };
}
#include "../../../_util.undef.h"