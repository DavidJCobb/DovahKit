#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class invalid_effect_type final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_effect_type(form_stub& subject, size_t i, uint8_t t) : base_form_load_warning(subject), which_effect(i), seen_type(t) {}

         size_t which_effect;
         uint8_t seen_type;
   };
}
#include "../../../_util.undef.h"