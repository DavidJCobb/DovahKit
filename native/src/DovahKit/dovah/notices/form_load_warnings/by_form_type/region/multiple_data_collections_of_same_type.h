#pragma once
#include "../../../base_form_load_warning.h"
#include "dovah/data/region_data_type.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::region {
   class multiple_data_collections_of_same_type : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr multiple_data_collections_of_same_type(
            form_stub& stub,
            region_data_type type,
            size_t count
         )
         :
            base_form_load_warning(stub),
            count(count),
            type(type)
         {}

         size_t count;
         region_data_type type;
   };
}
#include "../../../_util.undef.h"