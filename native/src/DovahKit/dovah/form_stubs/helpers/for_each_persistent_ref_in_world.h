#pragma once
#include <type_traits>
#include "../../form_stub.h"
#include "../../form_types.h"
#include "./for_each_child_form.h"
#include "./is_persistent.h"

namespace dovah::form_stub_helpers {
   //
   // Run a functor on every dialogue topic in the given quest. If your functor 
   // has a boolean return type, then returning true will break out of the loop 
   // early.
   //
   template<typename Functor> requires std::is_invocable_v<Functor, form_stub*>
   void for_each_persistent_ref_in_world(const form_stub& world, Functor&& functor) {
      if (world.form_type != form_type::worldspace)
         return;
      if constexpr (std::is_invocable_r_v<bool, Functor, form_stub*>) {
         for_each_child_form(&world, [&functor](form_stub* cell) {
            if (cell->form_type != form_type::cell)
               return false;
            bool result = false;
            for_each_child_form(cell, [&functor, &result](form_stub* child) {
               if (!is_persistent(child))
                  return false;
               result = (functor)(child);
               return result;
            });
            return result;
         });
      } else {
         for_each_child_form(&world, [&functor](form_stub* cell) {
            if (cell->form_type != form_type::cell)
               return;
            for_each_child_form(cell, [&functor](form_stub* child) {
               if (is_persistent(child))
                  functor(child);
            });
         });
      }
   }
}