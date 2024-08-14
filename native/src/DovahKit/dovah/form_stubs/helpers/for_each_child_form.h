#pragma once
#include <type_traits>
#include "../../form_stub.h"
#include "../../form_types.h"
#include "../../use_info_entry.h"

namespace dovah::form_stub_helpers {
   //
   // Run a functor on every child form of the given `parent`. If your functor 
   // has a boolean return type, then returning true will break out of the loop 
   // early.
   //
   template<typename Functor> requires std::is_invocable_v<Functor, form_stub*>
   void for_each_child_form(const form_stub* parent, Functor&& functor) {
      if (!parent)
         return;
      if (!(form_type_info::lookup(parent->form_type).flags & form_type_info::flag::can_have_children))
         return;
      for (auto& pair : parent->inbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::parent_child) {
            auto* child = entry.other;
            if (!child)
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