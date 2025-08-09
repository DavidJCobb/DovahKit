#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::region {
   class region_data_object_has_invalid_parent : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr region_data_object_has_invalid_parent(
            form_stub& stub,
            uint32_t which,
            uint32_t parent
         )
         :
            base_form_load_warning(stub),
            which(which),
            parent(parent)
         {}

         uint32_t which;
         uint32_t parent;
   };
}
#include "../../../_util.undef.h"