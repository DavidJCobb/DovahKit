#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class orphaned_entry_point_conditions : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr orphaned_entry_point_conditions(
            form_stub& stub,
            size_t which_effect,
            size_t count
         )
         :
            base_form_load_warning(stub),
            count(count),
            which_effect(which_effect)
         {}

         size_t count;
         size_t which_effect;
   };
}
#include "../../../_util.undef.h"