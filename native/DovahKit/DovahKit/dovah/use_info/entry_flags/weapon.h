#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class weapon : underlying_type {
      template_form = first_form_type_specific_flag, // WEAP/CNAM
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::weapon> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::weapon> {
      static constexpr const auto value = std::array{ form_type::weapon };
   };
}