#pragma once
#include <cstdint>

namespace dovah {
   enum class entry_point_value_type {
      none,
      activate_choice,
      leveled_item,
      localized_string,
      number,
      spell,
      raw_string,
   };

   enum class entry_point_function_type : uint8_t { // PERK/EPFT
      none                = 0,
      one_float           = 1,
      two_floats          = 2, // two floating-point values. the first may be an AV index stored as a float.
      leveled_item        = 3,
      activate_choice     = 4, // SPEL,LSTRING,flags
      spell               = 5,
      animation_graph_var = 6,
      localized_string    = 7,
   };

   constexpr entry_point_value_type entry_point_value_type_of(entry_point_function_type ft) {
      switch (ft) {
         case entry_point_function_type::activate_choice:
            return entry_point_value_type::activate_choice;
         case entry_point_function_type::animation_graph_var:
            return entry_point_value_type::raw_string;
         case entry_point_function_type::leveled_item:
            return entry_point_value_type::leveled_item;
         case entry_point_function_type::localized_string:
            return entry_point_value_type::localized_string;
         case entry_point_function_type::one_float:
         case entry_point_function_type::two_floats:
            return entry_point_value_type::number;
         case entry_point_function_type::spell:
            return entry_point_value_type::spell;
      }
      return entry_point_value_type::none;
   }

   enum class entry_point_function : uint8_t {
      none,
      set_value,                 // entry_point_function_type::one_float
      add_value,                 // entry_point_function_type::one_float
      multiply_value,            // entry_point_function_type::one_float
      add_range_to_value,        // entry_point_function_type::two_floats
      add_actor_value_mult,      // entry_point_function_type::two_floats
      absolute_value,            // entry_point_function_type::none
      negative_absolute_value,   // entry_point_function_type::none
      add_leveled_list,          // entry_point_function_type::leveled_item
      add_activate_choice,       // entry_point_function_type::activate_choice
      select_spell,              // entry_point_function_type::spell
      select_text,               // entry_point_function_type::animation_graph_var
      set_to_actor_value_mult,   // entry_point_function_type::two_floats
      multiply_actor_value_mult, // entry_point_function_type::two_floats
      multiply_one_plus_av_mult, // entry_point_function_type::two_floats
      set_text,                  // entry_point_function_type::localized_string
   };

   constexpr entry_point_function_type expected_type_for_entry_point_function(entry_point_function f) {
      switch (f) {
         case entry_point_function::none:
         case entry_point_function::absolute_value:
         case entry_point_function::negative_absolute_value:
            return entry_point_function_type::none;

         case entry_point_function::set_value:
         case entry_point_function::add_value:
         case entry_point_function::multiply_value:
            return entry_point_function_type::one_float;

         case entry_point_function::add_range_to_value:
         case entry_point_function::add_actor_value_mult:
         case entry_point_function::set_to_actor_value_mult:
         case entry_point_function::multiply_actor_value_mult:
         case entry_point_function::multiply_one_plus_av_mult:
            return entry_point_function_type::two_floats;

         case entry_point_function::add_leveled_list:
            return entry_point_function_type::leveled_item;

         case entry_point_function::add_activate_choice:
            return entry_point_function_type::activate_choice;

         case entry_point_function::select_spell:
            return entry_point_function_type::spell;

         case entry_point_function::select_text:
            return entry_point_function_type::animation_graph_var;

         case entry_point_function::set_text:
            return entry_point_function_type::localized_string;
      }
      if (std::is_constant_evaluated()) {
         throw; // fail on unrecognized values, during constant evaluation
      }
      return entry_point_function_type::none;
   }

   constexpr entry_point_value_type entry_point_value_type_of(entry_point_function ft) {
      return entry_point_value_type_of(expected_type_for_entry_point_function(ft));
   }

   constexpr bool entry_point_function_takes_an_av(entry_point_function f) {
      switch (f) {
         case entry_point_function::add_actor_value_mult:
         case entry_point_function::set_to_actor_value_mult:
         case entry_point_function::multiply_actor_value_mult:
         case entry_point_function::multiply_one_plus_av_mult:
            return true;
      }
      return false;
   }
}