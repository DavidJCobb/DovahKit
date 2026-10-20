#pragma once
#include "./base_extra_data.h"

namespace dovah::use_info::entry_flags {
   enum class reference : underlying_type {
      base_form = first_non_extra_data_flag, // REFR/NAME
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::reference> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::reference> {
      static constexpr const auto value = std::array{ form_type::reference };
   };
}