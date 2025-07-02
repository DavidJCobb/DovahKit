#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"
#include "dovah/data/packages/interrupt_override_target.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::faction {
   //
   // Package locations are used almost entirely in PACK forms... but Bethesda 
   // also uses them for Vendor Locations in FACT forms. 
   //
   class interrupt_override_target_not_in_a_package final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr interrupt_override_target_not_in_a_package(
            form_stub& subject,
            packages::interrupt_override_target target
         )
         :
            base_form_load_warning(subject),
            target(target)
         {}

         packages::interrupt_override_target target;
   };
}
#include "../../../_util.undef.h"