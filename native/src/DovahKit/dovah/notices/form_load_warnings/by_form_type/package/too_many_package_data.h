#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class too_many_package_data final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr too_many_package_data(
            form_stub& subject,
            size_t count
         )
         :
            base_form_load_warning(subject),
            count(count)
         {}

         size_t count;
   };
}
#include "../../../_util.undef.h"