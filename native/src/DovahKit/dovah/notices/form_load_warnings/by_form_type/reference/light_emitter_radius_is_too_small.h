#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::reference {
   class light_emitter_radius_is_too_small : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr light_emitter_radius_is_too_small(
            form_stub& stub,
            float minimum,
            float actual
         )
         :
            base_form_load_warning(stub),
            minimum(minimum),
            actual(actual)
         {}

         float minimum = 0;
         float actual  = 0;
   };
}
#include "../../../_util.undef.h"