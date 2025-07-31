#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"
#include "../../../../data/packages/legacy_type.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   //
   // The legacy package type in PKDT was incorrect: a custom-template-type package had 
   // a template. We've forced the legacy package type to "custom" instead of "custom 
   // template."
   //
   class package_with_a_template_cannot_itself_be_a_template final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr package_with_a_template_cannot_itself_be_a_template(
            form_stub& subject
         )
         :
            base_form_load_warning(subject)
         {}
   };
}
#include "../../../_util.undef.h"