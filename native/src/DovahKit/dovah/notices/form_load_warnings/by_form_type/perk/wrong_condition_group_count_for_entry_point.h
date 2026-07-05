#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"
#include "../../../../data/perk_entry_points.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class wrong_condition_group_count_for_entry_point final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr wrong_condition_group_count_for_entry_point(
            form_stub& subject,
            size_t which_effect,
            perk_entry_point entry_point,
            size_t actual_count,
            size_t declared_count,
            size_t expected_count
         )
         :
            base_form_load_warning(subject),
            which_effect(which_effect),
            entry_point(entry_point),
            condition_group_counts({ actual_count, declared_count, expected_count })
         {}

         size_t which_effect;
         perk_entry_point entry_point;
         struct {
            size_t actual;   // number of condition groups actually loaded (i.e. whichever is higher of: the number declared in DATA; or the highest-loaded PRKC, plus 1)
            size_t declared; // number of condition groups declared within DATA
            size_t expected; // number of condition groups that this entry point is known to use
         } condition_group_counts;
   };
}
#include "../../../_util.undef.h"