#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::race {
   class movement_type_override_without_speeds : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr movement_type_override_without_speeds(
            form_stub& stub
         )
         :
            base_form_load_warning(stub)
         {}
   };
}
#include "../../../_util.undef.h"