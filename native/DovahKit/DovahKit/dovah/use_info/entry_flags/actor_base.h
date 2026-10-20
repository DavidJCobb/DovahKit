#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class actor_base : underlying_type {
      template_actor = first_form_type_specific_flag, // NPC_/TPLT
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::actor_base> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::actor_base> {
      static constexpr const auto value = std::array{ form_type::actor_base };
   };
}