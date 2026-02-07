#include "./condition.h"
#include "./collection_conditions.h"
#include "helpers/lua/error.h"
#include "helpers/string/strieq_ascii.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/data/conditions/all_function_info.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/forms/components/conditions.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/Quest.h"
#include "editor/core.h"

#include "../form/form.h"
#include "../form/quest/alias.h"

#include "./condition/comparison.h"
#include "./condition/parameter_set.h"

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

context_type cls::context_of(wrapper& w) {
   auto* form = w.get_loaded_form_data<dovah::loaded_forms::Form>();
   if (!form)
      return {};
   return context_type(form->stub);
}

namespace {
   wrapped_type* _unwrap(wrapper& w) {
      return cls::unwrap(w);
   }
   context_type _context_of(wrapper& w) {
      return cls::context_of(w);
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
      int parameters(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::condition_parameters);
         return core::subsystems::userdata::get().push(L, out, wrappers::condition_parameter_set::metatable_key);
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
               if (auto* quest = _context_of(self).quest) {
                  return wrappers::quest_alias::wrap(L, quest, wrapped->get_run_on_data().index);
               }
               break;
            case dovah::conditions::run_on_type::package_data:
               lua_pushnumber(L, wrapped->get_run_on_data().index); // TODO: push a package-data object instead
               return 1;
            case dovah::conditions::run_on_type::reference:
               {
                  auto* stub = wrapped->get_run_on_data().reference.get_form_stub();
                  if (stub && stub->formID == dovah::hardcoded_form_ids::PlayerRef) {
                     lua_pushstring(L, "player");
                     return 1;
                  }
               }
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
      int use_aliases_for_params(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         lua_pushboolean(L, wrapped->test_flags(wrapped_type::flag::use_aliases));
         return 1;
      }
      int use_packdata_for_params(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         lua_pushboolean(L, wrapped->test_flags(wrapped_type::flag::use_package_data));
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
         if constexpr (std::is_invocable<CheckFunctor, wrapper&>) {
            check(self);
         } else {
            check();
         }
         self.before_edit();
         {
            working_type working(*wrapped);
            f(working);
            wrapped->commit(*form, working);
         }
         self.after_edit();
      }
      
      int function(lua_State* L) {
         uint16_t function_id = 0;
         _try_edit_condition(
            [L, &function_id]() {
               if (lua_isinteger(L, 2)) {
                  auto i = lua_tointeger(L, 2);
                  if (i < 0 || i >= std::numeric_limits<decltype(function_id)>::max()) {
                     cobb::lua::argerror(L, 2, "invalid integer");
                  }
                  const auto* info = dovah::conditions::function_info_by_id(i);
                  if (!info) {
                     cobb::lua::argerror(L, 2, "unrecognized function ID number");
                  }
                  function_id = i;
               } else if (lua_isstring(L, 2)) {
                  std::string_view arg = lua_tostring(L, 2);

                  bool found = false;
                  for (const auto& info : dovah::conditions::all_vanilla_function_info) {
                     if (cobb::strieq_ascii(info.name, arg)) {
                        function_id = info.id;
                        found       = true;
                        break;
                     }
                  }
                  if (found)
                     return;
                  for (const auto& info : dovah::conditions::all_extended_function_info) {
                     if (cobb::strieq_ascii(info.name, arg)) {
                        function_id = info.id;
                        found       = true;
                        break;
                     }
                  }
                  if (found)
                     return;
                  cobb::lua::argerror(L, 2, "unrecognized function name");
               } else {
                  cobb::lua::argerror(L, 2, "expected string (function name) or integer (function ID)");
               }
            },
            [L, &function_id](working_type& working) {
               if (working.function == function_id)
                  return;
               working.function = function_id;
               working.parameters = {};
               working.event_parameters = {};

               const auto* info = dovah::conditions::function_info_by_id(function_id);
               if (info && info->uses_event_data)
                  working.event_parameters.emplace();
            }
         );
         return 0;
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
      int run_on(lua_State* L) {
         dovah::conditions::run_on_type type;
         dovah::form_stub* ref = nullptr;
         int32_t index = -1;
         _try_edit_condition(
            [L, &type, &ref, &index](wrapper& self) {
               if (lua_isstring(L, 2)) {
                  std::string_view arg = lua_tostring(L, 2);
                  if (arg == "combat target") {
                     type = dovah::conditions::run_on_type::combat_target;
                  } else if (arg == "linked ref") {
                     type = dovah::conditions::run_on_type::linked_ref;
                  } else if (arg == "player") {
                     type = dovah::conditions::run_on_type::reference;
                     auto& editor = DovahKitCore::get();
                     if (!editor.has_data())
                        cobb::lua::error(L, "cannot create this data because no data is loaded in the editor");
                     ref = editor.get_form(dovah::hardcoded_form_ids::PlayerRef);
                     if (!ref)
                        cobb::lua::error(L, "an internal error occurred: could not locate PlayerRef");
                  } else if (arg == "subject") {
                     type = dovah::conditions::run_on_type::subject;
                  } else if (arg == "target") {
                     type = dovah::conditions::run_on_type::target;
                  } else {
                     cobb::lua::argerror(L, 2, "unrecognized run-on type");
                  }
               } else {
                  auto  context       = _context_of(self);
                  auto* alias_wrapper = wrapper_from_stack<wrappers::quest_alias>(L, 2);
                  if (alias_wrapper) {
                     auto* alias = wrappers::quest_alias::unwrap(*alias_wrapper);
                     if (!alias)
                        cobb::lua::argerror(L, 2, "passed-in quest alias is missing (deleted?)");
                     if (!context.quest)
                        cobb::lua::argerror(L, 2, "this condition has no owning quest, and so cannot be set to run on a quest alias");
                     if (&alias->owner.stub != context.quest)
                        cobb::lua::argerror(L, 2, "the passed-in quest alias does not belong to this condition's owning quest");
                     index = alias->id;
                     type  = dovah::conditions::run_on_type::quest_alias;
                  } else {
                     auto* form_wrapper = wrapper_from_stack<wrappers::form>(L, 2);
                     if (!form_wrapper) {
                        cobb::lua::argerror(L, 2, "form or string expected");
                     }
                     form_wrapper->error_if_wrong_form_type(L, 2, dovah::form_type::reference);
                     ref  = form_wrapper->stub;
                     type = dovah::conditions::run_on_type::reference;
                  }
                  // 
                  // TODO: in the future, handle package data and event data
                  //
               }
            },
            [L, &type, &ref, &index](working_type& working) {
               working.run_on.type = type;
               if (ref) {
                  working.run_on.entity.emplace<dovah::form_stub*>(ref);
               } else {
                  working.run_on.entity.emplace<uint32_t>(index);
               }
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
      int use_aliases_for_params(lua_State* L) {
         _try_edit_condition(
            [L]() {
               cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
            },
            [L](working_type& working) {
               cobb::edit_bit(working.flags, wrapped_type::flag::use_aliases, lua_toboolean(L, 2));
            }
         );
         return 0;
      }
      int use_packdata_for_params(lua_State* L) {
         _try_edit_condition(
            [L]() {
               cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
            },
            [L](working_type& working) {
               cobb::edit_bit(working.flags, wrapped_type::flag::use_package_data, lua_toboolean(L, 2));
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
      { "parameters",              &_getters::parameters },
      { "run_on",                  &_getters::run_on },
      { "swap_subject_and_target", &_getters::swap_subject_and_target },
      { "use_aliases_for_params",  &_getters::use_aliases_for_params },
      { "use_packdata_for_params", &_getters::use_packdata_for_params },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "function",                &_setters::function },
      { "is_or_linked",            &_setters::is_or_linked },
      { "run_on",                  &_setters::run_on },
      { "swap_subject_and_target", &_setters::swap_subject_and_target },
      { "use_aliases_for_params",  &_setters::use_aliases_for_params },
      { "use_packdata_for_params", &_setters::use_packdata_for_params },
   };
}