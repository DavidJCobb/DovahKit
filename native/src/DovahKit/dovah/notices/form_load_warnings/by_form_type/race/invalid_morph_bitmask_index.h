#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::race {
   class invalid_morph_bitmask_index : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_morph_bitmask_index(
            form_stub& stub,
            size_t     index
         )
         :
            base_form_load_warning(stub),
            index(index)
         {}

         size_t index;
         size_t max_index = 4;
   };
}
#include "../../../_util.undef.h"