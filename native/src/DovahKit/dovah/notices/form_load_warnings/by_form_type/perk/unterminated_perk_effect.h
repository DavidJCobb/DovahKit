#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class unterminated_perk_effect final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unterminated_perk_effect(form_stub& subject, size_t i) : base_form_load_warning(subject), which_effect(i) {}

         size_t which_effect;
   };
}
#include "../../../_util.undef.h"