#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::actor_value_info {
   //
   // Actor Value is not a skill, but has skill data. This data will be discarded.
   //
   class unexpected_skill_info final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unexpected_skill_info(form_stub& subject) : base_form_load_warning(subject) {}
   };
}
#include "../../../_util.undef.h"