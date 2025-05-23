#pragma once
#include <cstdint>
#include <optional>
#include "../base_form_load_warning.h"

#include "../_util.define.h"
namespace dovah::notices::form_load_warnings {
   class invalid_actor_value_index : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr invalid_actor_value_index(form_stub& subject, int32_t i) : base_form_load_warning(subject), actor_value(i) {}

         int32_t actor_value = 0;
   };
}
#include "../_util.undef.h"