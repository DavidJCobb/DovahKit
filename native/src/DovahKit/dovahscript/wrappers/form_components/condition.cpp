#include "./condition.h"
#include "./collection_conditions.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/data/conditions/all_function_info.h"
#include "dovah/forms/components/conditions.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "dovah/forms/Form.h"

#include "./condition/comparison.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::condition;
   using wrapped_type = cls::wrapped_type;
   using working_type = dovah::loaded_forms::components::conditions::working_condition;
   using context_type = dovah::loaded_forms::components::conditions::context;
}

wrapped_type* cls::unwrap(wrapper& w) {
   if (w.is_collection)
      return nullptr;
   auto* list_ptr = wrappers::collections::unwrap_condition_list(w);
   if (!list_ptr)
      return nullptr;

   size_t depth = w.parts.size();
   for (size_t i = 0; i < w.parts.size(); ++i) {
      if (w.parts[i].signature == wrapper_part_types::condition_list) {
         depth = i;
         break;
      }
   }
   if (depth >= w.parts.size())
      return nullptr;

   auto& list = *list_ptr;
   auto  i = w.parts[depth].index;
   if (i >= list.size())
      return nullptr;
   return &list[i];
}

namespace {
   wrapped_type* _unwrap(wrapper& w) {
      return cls::unwrap(w);
   }
   context_type _context_of(wrapper& w) {
      auto* form = w.get_loaded_form_data<dovah::loaded_forms::Form>();
      if (!form)
         return {};
      return context_type(form->stub);
   }

   namespace _getters {
      int comparison(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::condition_comparison);
         return core::subsystems::userdata::get().push(L, out, wrappers::condition_comparison::metatable_key);
      }
      int function(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");

         if (auto* info = wrapped->get_function()) {
            lua_pushstring(L, info->name.data());
         } else {
            lua_pushnumber(L, wrapped->get_function_id());
         }
         return 1;
      }
      int owning_package(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         auto  context = _context_of(self);
         return push_native_object(context.package);
      }
      int owning_quest(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         auto  context = _context_of(self);
         return push_native_object(context.quest);
      }
      int is_or_linked(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         lua_pushboolean(L, wrapped->test_flags(wrapped_type::flag::or_linked));
         return 1;
      }
      int run_on(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");

         switch (wrapped->get_run_on_data().type) {
            case dovah::conditions::run_on_type::combat_target:
               lua_pushstring(L, "combat target");
               return 1;
            case dovah::conditions::run_on_type::event_data:
               lua_pushnumber(L, wrapped->get_run_on_data().index); // TODO: push an event-data object instead
               return 1;
            case dovah::conditions::run_on_type::linked_ref:
               lua_pushstring(L, "linked ref");
               return 1;
            case dovah::conditions::run_on_type::quest_alias:
               lua_pushnumber(L, wrapped->get_run_on_data().index); // TODO: push a quest-alias object instead
               return 1;
            case dovah::conditions::run_on_type::package_data:
               lua_pushnumber(L, wrapped->get_run_on_data().index); // TODO: push a package-data object instead
               return 1;
            case dovah::conditions::run_on_type::reference:
               return push_native_object(wrapped->get_run_on_data().reference);
            case dovah::conditions::run_on_type::subject:
               lua_pushstring(L, "subject");
               return 1;
            case dovah::conditions::run_on_type::target:
               lua_pushstring(L, "target");
               return 1;
         }
         lua_pushnil(L);
         return 1;
      }
      int swap_subject_and_target(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         lua_pushboolean(L, wrapped->test_flags(wrapped_type::flag::swap_subject_and_target));
         return 1;
      }
   }
   namespace _setters {
      template<typename CheckFunctor, typename EditFunctor>
      void _try_edit_condition(CheckFunctor&& check, EditFunctor&& f) {
         core::subsystems::permissions::verify_form_write_permissions();
         
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* form    = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         check();
         self.before_edit();
         {
            working_type working(*wrapped);
            f(working);
            wrapped->commit(*form, working);
         }
         self.after_edit();
      }

      int is_or_linked(lua_State* L) {
         _try_edit_condition(
            [L]() {
               cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
            },
            [L](working_type& working) {
               cobb::edit_bit(working.flags, wrapped_type::flag::or_linked, lua_toboolean(L, 2));
            }
         );
         return 0;
      }
      int swap_subject_and_target(lua_State* L) {
         _try_edit_condition(
            [L]() {
               cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
            },
            [L](working_type& working) {
               cobb::edit_bit(working.flags, wrapped_type::flag::swap_subject_and_target, lua_toboolean(L, 2));
            }
         );
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "comparison",              &_getters::comparison },
      { "function",                &_getters::function },
      { "is_or_linked",            &_getters::is_or_linked },
      { "owning_package",          &_getters::owning_package },
      { "owning_quest",            &_getters::owning_quest },
      { "run_on",                  &_getters::run_on },
      { "swap_subject_and_target", &_getters::swap_subject_and_target },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "is_or_linked",            &_setters::is_or_linked },
      { "swap_subject_and_target", &_setters::swap_subject_and_target },
   };
}