#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::reference {
   class map_marker_has_no_data : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr map_marker_has_no_data(
            form_stub& stub
         )
         :
            base_form_load_warning(stub)
         {}
   };
}
#include "../../../_util.undef.h"