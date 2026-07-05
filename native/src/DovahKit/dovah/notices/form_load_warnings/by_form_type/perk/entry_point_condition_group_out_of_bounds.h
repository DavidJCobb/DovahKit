#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class entry_point_condition_group_out_of_bounds : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr entry_point_condition_group_out_of_bounds(
            form_stub& stub,
            size_t which_effect,
            size_t group_index,
            size_t expected_group_count
         )
         :
            base_form_load_warning(stub),
            group_index(group_index),
            expected_group_count(expected_group_count)
         {}

         size_t which_effect;
         size_t group_index;
         size_t expected_group_count;
   };
}
#include "../../../_util.undef.h"