#pragma once
#include <type_traits>
#include "../../form_stub.h"
#include "../../use_info/entry.h"
#include "../../use_info/entry_flag_to_mask.h"
#include "../../use_info/entry_flag_type_pertains_to_form_type.h"

namespace dovah::form_stub_helpers {
   template<auto Flag, typename Functor>
      requires (std::is_invocable_v<Functor, form_stub&> && use_info::is_entry_flag_type_v<decltype(Flag)>)
   void for_each_inbound_use_with_flag(const form_stub& used, Functor&& functor) {
      constexpr const auto entry_flag_mask = use_info::entry_flag_to_mask(Flag);
      for (auto& pair : used.inbound) {
         auto& entry = pair.second;
         if (entry.flags & entry_flag_mask) {
            auto* user = entry.other;
            if (!user || !use_info::entry_flag_type_pertains_to_form_type<decltype(Flag)>(user->form_type))
               continue;
            if constexpr (std::is_invocable_r_v<bool, Functor, form_stub&>) {
               if (functor(*user))
                  break;
            } else {
               functor(*user);
            }
         }
      }
   }
}