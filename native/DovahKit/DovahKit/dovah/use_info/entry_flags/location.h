#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class location : underlying_type {
      parent_location = first_form_type_specific_flag, // LCTN/PNAM
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::location> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::location> {
      static constexpr const auto value = std::array{ form_type::location };
   };
}