#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::reference {
   class actor_reflected_by_water_refs : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr actor_reflected_by_water_refs(
            form_stub& stub,
            size_t     count
         )
         :
            base_form_load_warning(stub),
            count(count)
         {}

         size_t count = 0;
   };
}
#include "../../../_util.undef.h"