#pragma once
#include <type_traits>
#include "../../form_stub.h"
#include "../../use_info/entry.h"
#include "../../use_info/entry_flag_to_mask.h"
#include "../../use_info/entry_flag_type_pertains_to_form_type.h"
#include "../../use_info/entry_flag_underlying_type.h"
#include "../../use_info/is_entry_flag_type.h"
namespace dovah::use_info::entry_flags {
   enum class base : entry_flag_underlying_type;
   enum class base_extra_data : entry_flag_underlying_type;
}

namespace dovah::form_stub_helpers {
   template<auto Flag> requires use_info::is_entry_flag_type_v<decltype(Flag)>
   form_stub* get_unique_outbound_use(const form_stub& user) {
      using flag_type = decltype(Flag);
      if (!use_info::entry_flag_type_pertains_to_form_type<flag_type>(user.form_type)) {
         return nullptr;
      }
      constexpr const auto flags_mask = use_info::entry_flag_to_mask(Flag);
      for (auto& pair : user.outbound) {
         auto& entry = pair.second;
         if (entry.flags & flags_mask)
            return entry.other;
      }
      return nullptr;
   }
}