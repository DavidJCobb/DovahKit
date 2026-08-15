#pragma once
#include <array>
#include <cstdint>
#include <variant>
#include <vector>
#include <QString>
#include "dovah/data/entry_point_functions.h"
#include "dovah/data/perk_entry_points.h"
#include "ui/types/conditions/condition.h"
namespace dovah {
   class form_stub;
}

namespace ui::types::perk_entries {
   namespace params {
      struct activate_choice final {
         bool operator==(const activate_choice&) const noexcept = default;
         QString           label;
         dovah::form_stub* spell = nullptr;
         bool run_immediately = false;
         bool replace_default = false;
         struct _ {
            // "struct underscore..." ugh. it's so stupid that anonymous structs in a 
            // defaulted-equality struct don't themselves have equality defaulted.
            bool operator==(const _&) const noexcept = default;
            //
            QString script;
            QString function;
         } fragment;
      };
      struct form final {
         constexpr bool operator==(const form&) const noexcept = default;
         dovah::form_stub* value = nullptr;
      };
      struct one_float final {
         constexpr bool operator==(const one_float&) const noexcept = default;
         float value = 0;
      };
      struct one_av_one_float final {
         constexpr bool operator==(const one_av_one_float&) const noexcept = default;
         int32_t actor_value = -1;
         float   value       =  0;
      };
      struct two_floats final {
         constexpr bool operator==(const two_floats&) const noexcept = default;
         std::array<float, 2> values = { 0, 0 };
      };
      struct raw_string final {
         bool operator==(const raw_string&) const noexcept = default;
         QString value;
      };
      struct localized_string final {
         bool operator==(const localized_string&) const noexcept = default;
         QString value;
      };
   }

   class entry_point_entry {
      public:
         constexpr bool operator==(const entry_point_entry&) const noexcept = default;

         using condition_group = std::vector<ui::types::conditions::condition>;

      public:
         dovah::perk_entry_point     entry_point = (dovah::perk_entry_point)dovah::all_perk_entry_points.size(); // deliberate out-of-range value
         dovah::entry_point_function function    = dovah::entry_point_function::none;
         std::variant<
            std::monostate,
            params::activate_choice,
            params::form,
            params::localized_string,
            params::one_av_one_float,
            params::one_float,
            params::raw_string,
            params::two_floats
         > parameters;
         std::vector<condition_group> conditions_by_entity;

      protected:
         void _set_function_type(dovah::entry_point_value_type);
      public:
         void set_entry_point(dovah::perk_entry_point);
         void set_function(dovah::entry_point_function);
   };
}