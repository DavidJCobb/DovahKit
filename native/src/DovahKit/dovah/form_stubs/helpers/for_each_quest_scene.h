#pragma once
#include <type_traits>
#include "../../form_stub.h"
#include "../../form_types.h"
#include "../../use_info/entry_flags/scene.h"
#include "./for_each_inbound_use_with_flag.h"

namespace dovah::form_stub_helpers {
   template<typename Functor> requires std::is_invocable_v<Functor, form_stub&>
   void for_each_quest_scene(const form_stub& quest, Functor&& functor) {
      if (quest.form_type != form_type::quest)
         return;
      for_each_inbound_use_with_flag<use_info::entry_flags::scene::parent_quest>(quest, functor);
   }
}