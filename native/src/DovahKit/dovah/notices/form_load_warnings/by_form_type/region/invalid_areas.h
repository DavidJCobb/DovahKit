#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::region {
   //
   // The region contains areas that have fewer than two vertices or 
   // are self-intersecting.
   //
   class invalid_areas : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_areas(
            form_stub& stub,
            size_t count
         )
         :
            base_form_load_warning(stub),
            count(count)
         {}

         size_t count;
   };
}
#include "../../../_util.undef.h"