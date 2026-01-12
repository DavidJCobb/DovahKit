#pragma once
#include <type_traits>
#include "../../form_stub.h"
#include "../../form_types.h"
#include "../../use_info/entry_flags/base.h"
#include "./for_each_inbound_use_with_flag.h"

namespace dovah::form_stub_helpers {
   //
   // Run a functor on every child form of the given `parent`. If your functor 
   // has a boolean return type, then returning true will break out of the loop 
   // early.
   //
   template<typename Functor> requires std::is_invocable_v<Functor, form_stub&>
   void for_each_child_form(const form_stub& parent, Functor&& functor) {
      if (!(form_type_info::lookup(parent.form_type).flags & form_type_info::flag::can_have_children))
         return;
      for_each_inbound_use_with_flag<use_info::entry_flags::base::parent>(parent, functor);
   }
}