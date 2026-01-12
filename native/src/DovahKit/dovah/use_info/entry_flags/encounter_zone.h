#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class encounter_zone : underlying_type {
      location = first_form_type_specific_flag, // ECZN/DATA+0x04
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::encounter_zone> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::encounter_zone> {
      static constexpr const auto value = std::array{ form_type::encounter_zone };
   };
}