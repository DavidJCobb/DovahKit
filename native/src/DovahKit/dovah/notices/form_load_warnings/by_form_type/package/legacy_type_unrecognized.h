#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"
#include "../../../../data/packages/legacy_type.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class legacy_type_unrecognized final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         using value_type = std::underlying_type_t<packages::legacy_type>;

      public:
         constexpr legacy_type_unrecognized(
            form_stub& subject,
            packages::legacy_type t
         )
         :
            base_form_load_warning(subject),
            seen((value_type)t)
         {}

         value_type seen;
   };
}
#include "../../../_util.undef.h"