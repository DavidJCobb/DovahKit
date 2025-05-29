#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::magic_effect {
   class counters_itself : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr counters_itself(
            form_stub& stub
         )
         :
            base_form_load_warning(stub)
         {}
   };
}
#include "../../../_util.undef.h"