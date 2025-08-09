#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::region {
   class unknown_region_data_type : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unknown_region_data_type(
            form_stub& stub,
            uint32_t type
         )
         :
            base_form_load_warning(stub),
            type(type)
         {}

         uint32_t type;
   };
}
#include "../../../_util.undef.h"