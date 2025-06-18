#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class effect_header_has_invalid_size : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr effect_header_has_invalid_size(
            form_stub& stub,
            size_t which_effect,
            size_t size_seen,
            size_t size_expected
         )
         :
            base_form_load_warning(stub),
            which_effect(which_effect),
            size_seen(size_seen),
            size_expected(size_expected)
         {}

         size_t which_effect;
         size_t size_seen;
         size_t size_expected;
   };
}
#include "../../../_util.undef.h"