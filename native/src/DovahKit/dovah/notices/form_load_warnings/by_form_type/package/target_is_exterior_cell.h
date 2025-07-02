#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class target_is_exterior_cell final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

         using value_type = std::underlying_type_t<packages::interrupt_override_target>;

      public:
         constexpr target_is_exterior_cell(
            form_stub& subject,
            form_stub& cell
         )
         :
            base_form_load_warning(subject),
            cell(cell)
         {}

         form_stub& cell;
   };
}
#include "../../../_util.undef.h"