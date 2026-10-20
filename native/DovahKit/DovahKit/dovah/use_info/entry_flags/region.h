#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class region : underlying_type {
      worldspace = first_form_type_specific_flag, // REGN/WNAM
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::region> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::region> {
      static constexpr const auto value = std::array{ form_type::region };
   };
}