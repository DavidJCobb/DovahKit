#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class no_ranks final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr no_ranks(form_stub& subject) : base_form_load_warning(subject) {}
   };
}
#include "../../../_util.undef.h"