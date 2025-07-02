#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"
#include "dovah/data/packages/interrupt_override_target.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class invalid_interrupt_override_target final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

         using value_type = std::underlying_type_t<packages::interrupt_override_target>;

      public:
         constexpr invalid_interrupt_override_target(
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