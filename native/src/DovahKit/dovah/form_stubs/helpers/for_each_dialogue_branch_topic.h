#pragma once
#include <type_traits>
#include "../../form_stub.h"
#include "../../form_types.h"
#include "../../use_info/entry.h"
#include "../../use_info/entry_flag_to_mask.h"
#include "../../use_info/entry_flags/topic.h"

namespace dovah::form_stub_helpers {
   //
   // Run a functor on every dialogue topic in the given dialogue branch. If your 
   // functor has a boolean return type, then returning true will break out of 
   // the loop early.
   // 
   // Note that this grabs all topics, including those that have a containing 
   // dialogue branch.
   //
   template<typename Functor> requires std::is_invocable_v<Functor, form_stub&>
   void for_each_dialogue_branch_topic(const form_stub& quest, Functor&& functor) {
      constexpr const auto entry_flag_mask = use_info::entry_flag_to_mask(use_info::entry_flags::topic::parent_branch);

      if (quest.form_type != form_type::dialogue_branch)
         return;
      for (auto& pair : quest.inbound) {
         auto& entry = pair.second;
         if (entry.flags & entry_flag_mask) {
            auto* child = entry.other;
            if (!child || child->form_type != form_type::topic)
               continue;
            if constexpr (std::is_invocable_r_v<bool, Functor, form_stub*>) {
               if (functor(*child))
                  break;
            } else {
               functor(*child);
            }
         }
      }
   }
}