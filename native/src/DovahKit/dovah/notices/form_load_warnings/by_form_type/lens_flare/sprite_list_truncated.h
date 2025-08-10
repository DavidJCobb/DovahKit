#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::lens_flare {
   class sprite_list_truncated : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr sprite_list_truncated(
            form_stub& stub,
            size_t count_expected,
            size_t count_seen
         )
         :
            base_form_load_warning(stub),
            count_expected(count_expected),
            count_seen(count_seen)
         {}

         size_t count_expected;
         size_t count_seen;
   };
}
#include "../../../_util.undef.h"