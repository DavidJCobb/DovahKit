#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"
#include "../../../../data/packages/legacy_type.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class package_changed_legacy_type_during_load final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr package_changed_legacy_type_during_load(
            form_stub& subject,
            packages::legacy_type from,
            packages::legacy_type to
         )
         :
            base_form_load_warning(subject),
            from(from),
            to(to)
         {}

         packages::legacy_type from;
         packages::legacy_type to;
   };
}
#include "../../../_util.undef.h"