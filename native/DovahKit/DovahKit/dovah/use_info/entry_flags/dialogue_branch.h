#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class dialogue_branch : underlying_type {
      parent_quest = first_form_type_specific_flag, // DLBR/QNAM
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::dialogue_branch> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::dialogue_branch> {
      static constexpr const auto value = std::array{ form_type::dialogue_branch };
   };
}