#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::region {
   class bad_region_point_list_data_subrecord_size : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr bad_region_point_list_data_subrecord_size(
            form_stub& stub,
            uint32_t subrecord,
            uint32_t size
         )
         :
            base_form_load_warning(stub),
            subrecord(subrecord),
            size(size)
         {}

         uint32_t subrecord;
         uint32_t size;
   };
}
#include "../../../_util.undef.h"