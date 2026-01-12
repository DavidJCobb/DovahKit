#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class armor : underlying_type {
      template_form = first_form_type_specific_flag, // ARMO/TNAM
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::armor> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::armor> {
      static constexpr const auto value = std::array{ form_type::armor };
   };
}