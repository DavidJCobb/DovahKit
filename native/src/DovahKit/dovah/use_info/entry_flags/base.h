#pragma once
#include "../entry_flag_underlying_type.h"
#include "../is_entry_flag_type.h"
#include "../entry_flag_type_pertains_to_form_type.h"
#include "../../form_types.h"

namespace dovah::use_info::entry_flags {
   using underlying_type = entry_flag_underlying_type;

   // Use info flags that are available on any user-form type.
   enum class base : underlying_type {
      // Indicates that the user-form's record is in a child GRUP of the used-form's record
      parent = 0,

      __COUNT
   };
   constexpr const size_t first_form_type_specific_flag = (size_t)base::__COUNT;
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::base> {
      static constexpr const bool value = true;
   };
}