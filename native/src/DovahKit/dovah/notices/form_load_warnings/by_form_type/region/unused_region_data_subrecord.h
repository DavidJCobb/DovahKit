#pragma once
#include "../../../base_form_load_warning.h"
#include "dovah/data/region_data_type.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::region {
   class unused_region_data_subrecord : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unused_region_data_subrecord(
            form_stub& stub,
            uint32_t subrecord,
            region_data_type type
         )
         :
            base_form_load_warning(stub),
            subrecord(subrecord),
            type(type)
         {}

         uint32_t subrecord;
         region_data_type type;
   };
}
#include "../../../_util.undef.h"