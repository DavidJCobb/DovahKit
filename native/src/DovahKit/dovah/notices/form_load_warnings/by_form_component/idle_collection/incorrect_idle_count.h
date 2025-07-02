#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::idle_collection {
   class incorrect_idle_count : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr incorrect_idle_count(
            form_stub& stub,
            size_t     count_expected,
            size_t     count_seen
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