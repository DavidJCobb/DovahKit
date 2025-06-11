#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::footstep_set {
   class footstep_count_mismatch : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr footstep_count_mismatch(
            form_stub& stub,
            std::array<size_t, 5> expected,
            size_t total_found
         )
         :
            base_form_load_warning(stub),
            expected(expected),
            total_found(total_found)
         {}

         std::array<size_t, 5> expected = {};
         size_t total_found = 0;
   };
}
#include "../../../_util.undef.h"