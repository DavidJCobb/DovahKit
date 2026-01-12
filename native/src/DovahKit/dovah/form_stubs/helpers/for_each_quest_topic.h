#pragma once
#include <type_traits>
#include "../../form_stub.h"
#include "../../form_types.h"
#include "../../use_info/entry_flags/topic.h"
#include "./for_each_inbound_use_with_flag.h"

namespace dovah::form_stub_helpers {
   //
   // Run a functor on every dialogue topic in the given quest. If your functor 
   // has a boolean return type, then returning true will break out of the loop 
   // early.
   // 
   // Note that this grabs all topics, including those that have a containing 
   // dialogue branch.
   //
   template<typename Functor> requires std::is_invocable_v<Functor, form_stub&>
   void for_each_quest_topic(const form_stub& quest, Functor&& functor) {
      if (quest.form_type != form_type::quest)
         return;
      for_each_inbound_use_with_flag<use_info::entry_flags::topic::parent_quest>(quest, functor);
   }
}