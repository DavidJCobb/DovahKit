#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class worldspace : underlying_type {
      parent_worldspace = first_form_type_specific_flag, // WRLD/WNAM
      encounter_zone, // WRLD/XEZN
      location,       // WRLD/XLCN
   };
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::worldspace> {
      static constexpr const bool value = true;
   };

   template<>
   struct entry_flag_type_form_types<entry_flags::worldspace> {
      static constexpr const auto value = std::array{ form_type::worldspace };
   };
}