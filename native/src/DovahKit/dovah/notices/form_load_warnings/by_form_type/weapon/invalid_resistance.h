#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::weapon {
   class invalid_resistance : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_resistance(
            form_stub& stub,
            uint32_t   actor_value
         )
         :
            base_form_load_warning(stub),
            actor_value(actor_value)
         {}

         uint32_t actor_value;
   };
}
#include "../../../_util.undef.h"