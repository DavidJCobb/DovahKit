#pragma once
#include "../form_types.h"
#include "./entry_flags/base.h"
#include "./entry_flags/base_extra_data.h"
#include "./FOR_EACH_ENTRY_FLAG_TYPE.define.h"

namespace dovah::use_info::entry_flags {
   #define X(name, ...) enum class name : entry_flag_underlying_type;
   FOR_EACH_ENTRY_FLAG_TYPE(X);
   #undef X
}

namespace dovah::use_info {
   template<auto UseInfoEntryFlag>
   constexpr const form_type form_type_for_entry_flag = []() -> form_type {
      using entry_flag_type = decltype(UseInfoEntryFlag);
      if constexpr (std::is_same_v<entry_flag_type, entry_flags::base>) {
         return form_type::none;
      } else if constexpr (std::is_same_v<entry_flag_type, entry_flags::base_extra_data>) {
         return form_type::none;
      }
      //
      #define X(name, ...) \
         else if constexpr (std::is_same_v<entry_flag_type, entry_flags::name>) { \
            return form_type::name; \
         }
      FOR_EACH_ENTRY_FLAG_TYPE(X)
      #undef X
      //
      else {
         throw;
      }
   }();
}

#include "./FOR_EACH_ENTRY_FLAG_TYPE.undef.h"