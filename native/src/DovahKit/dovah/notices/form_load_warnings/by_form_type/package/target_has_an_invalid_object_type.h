#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"
#include "dovah/data/packages/object_type.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class target_has_an_invalid_object_type final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

         using value_type = std::underlying_type_t<packages::object_type>;

      public:
         constexpr target_has_an_invalid_object_type(
            form_stub& subject,
            value_type seen
         )
         :
            base_form_load_warning(subject),
            seen(seen)
         {}

         value_type seen;
   };
}
#include "../../../_util.undef.h"