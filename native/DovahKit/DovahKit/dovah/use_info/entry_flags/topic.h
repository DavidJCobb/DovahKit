#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class topic : underlying_type {
      parent_branch = first_form_type_specific_flag, // DIAL/BNAM
      parent_quest, // DIAL/QNAM
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::topic> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::topic> {
      static constexpr const auto value = std::array{ form_type::topic };
   };
}