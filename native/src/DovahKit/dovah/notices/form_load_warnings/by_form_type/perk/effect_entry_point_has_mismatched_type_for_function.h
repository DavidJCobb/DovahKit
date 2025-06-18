#pragma once
#include "../../../base_form_load_warning.h"
#include "dovah/data/entry_point_functions.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class effect_entry_point_has_mismatched_type_for_function : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr effect_entry_point_has_mismatched_type_for_function(
            form_stub& stub,
            size_t which_effect,
            entry_point_function f,
            entry_point_function_type expected,
            entry_point_function_type seen
         )
         :
            base_form_load_warning(stub),
            which_effect(which_effect),
            function(f),
            type_expected(expected),
            type_seen(seen)
         {}

         size_t which_effect;
         entry_point_function function;
         entry_point_function_type type_expected;
         entry_point_function_type type_seen;
   };
}
#include "../../../_util.undef.h"