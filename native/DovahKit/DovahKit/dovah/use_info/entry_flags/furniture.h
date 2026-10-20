#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class furniture : underlying_type {
      water_type = first_form_type_specific_flag, // ACTI/WNAM
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::furniture> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::furniture> {
      static constexpr const auto value = std::array{ form_type::furniture };
   };
}