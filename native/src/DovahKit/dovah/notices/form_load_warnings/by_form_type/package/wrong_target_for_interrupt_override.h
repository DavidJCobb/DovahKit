#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"
#include "dovah/data/packages/interrupt_override_target.h"
#include "dovah/data/packages/interrupt_override_type.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class wrong_target_for_interrupt_override final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr wrong_target_for_interrupt_override(
            form_stub& subject,
            packages::interrupt_override_target target,
            packages::interrupt_override_type required_type,
            packages::interrupt_override_type actual_type
         )
         :
            base_form_load_warning(subject),
            target(target),
            required_type(required_type),
            actual_type(actual_type)
         {}

         packages::interrupt_override_target target;
         packages::interrupt_override_type required_type;
         packages::interrupt_override_type actual_type;
   };
}
#include "../../../_util.undef.h"