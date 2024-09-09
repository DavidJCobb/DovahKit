#pragma once
#include <type_traits>
#include "../../form_stub.h"
#include "../../form_types.h"
#include "../../use_info_entry.h"

namespace dovah::form_stub_helpers {
   //
   // Run a functor on every dialogue branch in the given quest. If your functor 
   // has a boolean return type, then returning true will break out of the loop 
   // early.
   //
   template<typename Functor> requires std::is_invocable_v<Functor, form_stub*>
   void for_each_quest_dialogue_branch(const form_stub* quest, Functor&& functor) {
      if (!quest || quest->form_type != form_type::quest)
         return;
      for (auto& pair : quest->inbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::dialogue_quest) {
            auto* child = entry.other;
            if (!child || child->form_type != form_type::dialogue_branch)
               continue;
            if constexpr (std::is_invocable_r_v<bool, Functor, form_stub*>) {
               if (functor(child))
                  break;
            } else {
               functor(child);
            }
         }
      }
   }
}