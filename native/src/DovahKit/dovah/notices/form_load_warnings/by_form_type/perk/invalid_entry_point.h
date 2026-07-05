#pragma once
#include <type_traits>
#include "../../../base_form_load_warning.h"
#include "../../../../data/perk_entry_points.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class invalid_entry_point final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         using value_type = std::underlying_type_t<dovah::perk_entry_point>;

      public:
         constexpr invalid_entry_point(form_stub& subject, size_t i, value_type t) : base_form_load_warning(subject), which_effect(i), seen_entry_point(t) {}

         size_t     which_effect;
         value_type seen_entry_point;
   };
}
#include "../../../_util.undef.h"