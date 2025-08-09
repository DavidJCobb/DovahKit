#pragma once
#include "../../../base_form_load_warning.h"
#include "dovah/data/region_data_type.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::region {
   class mismatched_region_data_subrecord : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr mismatched_region_data_subrecord(
            form_stub& stub,
            uint32_t subrecord,
            region_data_type type_seen,
            region_data_type type_expected
         )
         :
            base_form_load_warning(stub),
            subrecord(subrecord),
            type_seen(type_seen),
            type_expected(type_expected)
         {}

         uint32_t subrecord;
         region_data_type type_seen;
         region_data_type type_expected;
   };
}
#include "../../../_util.undef.h"