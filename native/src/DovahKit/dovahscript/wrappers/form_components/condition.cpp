#include "./condition.h"
#include "./collection_conditions.h"
#include <expected>
#include "helpers/function_traits.h"
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
#include "dovah/forms/TopicInfo.h"
#include "editor/core.h"

#include "../form/form.h"
#include "../form/quest/alias.h"

#include "./condition/comparison.h"
#include "./condition/parameter_set.h"
#include "./condition/impl/pull_condition_comparison_from_lua.h"
#include "./condition/impl/pull_condition_parameter_from_lua.h"
#include "./condition/impl/pull_condition_parameter_set_from_lua.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::condition;
   using wrapped_type = cls::wrapped_type;
   using working_type = dovah::loaded_forms::components::conditions::working_condition;
   using context_type = dovah::loaded_forms::components::conditions::context;

   using parameter_type_override = dovah::loaded_forms::components::conditions::parameter_type_override;
   using working_comparison      = decltype(working_type::comparison);
   using working_run_on_params   = decltype(working_type::run_on);
}

bool cls::is_not_locked(wrapper& w) {
   size_t depth = w.parts.size();
   for (size_t i = 0; i < w.parts.size(); ++i) {
      if (w.parts[i].signature == wrapper_part_types::condition_list) {
         depth = i;
         break;
      }
   }
   if (depth < w.parts.size()) {
      if (w.stub && w.stub->form_type == dovah::form_type::topic_info) {
         auto* form = w.get_loaded_form_data<dovah::loaded_forms::TopicInfo>();
         if (!form)
            return false;
         return w.parts[depth].index >= form->conditions.locked.size();
      }
   }
   return true;
}
wrapped_type* cls::unwrap(wrapper& w) {
   if (w.is_collection)
      return nullptr;

   auto [list_ptr, i] = wrappers::collections::unwrap_condition_list_and_index(w);
   if (!list_ptr)
      return nullptr;
   auto& list = *list_ptr;
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
   std::expected<uint16_t, std::string_view> function_id_from_lua(lua_State* L, int pos) {
      if (lua_isinteger(L, pos)) {
         const auto i = lua_tointeger(L, pos);
         if (i < 0 || i >= std::numeric_limits<uint16_t>::max()) {
            return std::unexpected("invalid integer");
         }
         const auto* info = dovah::conditions::function_info_by_id(i);
         if (!info) {
            return std::unexpected("unrecognized function ID number");
         }
         return i;
      }
      if (lua_isstring(L, pos)) {
         const std::string_view arg = lua_tostring(L, pos);
         for (const auto& info : dovah::conditions::all_vanilla_function_info)
            if (cobb::strieq_ascii(info.name, arg))
               return info.id;
         for (const auto& info : dovah::conditions::all_extended_function_info)
            if (cobb::strieq_ascii(info.name, arg))
               return info.id;
         return std::unexpected("unrecognized function name");
      }
      return std::unexpected("expected string (function name) or integer (function ID)");
   }
   std::expected<parameter_type_override, std::string_view> parameter_type_override_from_lua(lua_State* L, int pos) {
      if (!lua_isstring(L, pos))
         return std::unexpected("string expected");
      std::string_view v = lua_tostring(L, pos);
      if (v == "none") {
         return parameter_type_override::none;
      } else if (v == "alias") {
         return parameter_type_override::alias;
      } else if (v == "packdata") {
         return parameter_type_override::package_data;
      }
      return std::unexpected("unrecognized value");
   }
   std::expected<working_run_on_params, std::string_view> run_on_from_lua(lua_State* L, int pos, const context_type& context) {
      using run_on_params = working_run_on_params;

      if (lua_isstring(L, pos)) {
         std::string_view arg = lua_tostring(L, pos);
         if (arg == "combat target") {
            return run_on_params{ .type = dovah::conditions::run_on_type::combat_target };
         }
         if (arg == "linked ref") {
            return run_on_params{ .type = dovah::conditions::run_on_type::linked_ref };
         }
         if (arg == "player") {
            auto& editor = DovahKitCore::get();
            if (!editor.has_data())
               return std::unexpected("cannot set the run-on type to the player because no data is loaded in the editor");
            auto* ref = editor.get_form(dovah::hardcoded_form_ids::PlayerRef);
            if (!ref)
               return std::unexpected("an internal error occurred: could not locate PlayerRef");
            return run_on_params{
               .type   = dovah::conditions::run_on_type::reference,
               .entity = ref
            };
         }
         if (arg == "subject") {
            return run_on_params{ .type = dovah::conditions::run_on_type::subject };
         }
         if (arg == "target") {
            return run_on_params{ .type = dovah::conditions::run_on_type::target };
         }
         return std::unexpected("unrecognized run-on type");
      }
      auto* alias_wrapper = wrapper_from_stack<wrappers::quest_alias>(L, pos);
      if (alias_wrapper) {
         auto* alias = wrappers::quest_alias::unwrap(*alias_wrapper);
         if (!alias)
            return std::unexpected("passed-in quest alias is missing (deleted?)");
         if (!context.quest)
            return std::unexpected("this condition has no owning quest, and so cannot be set to run on a quest alias");
         if (&alias->owner.stub != context.quest)
            return std::unexpected("the passed-in quest alias does not belong to this condition's owning quest");
         return run_on_params{
            .type   = dovah::conditions::run_on_type::quest_alias,
            .entity = alias->id
         };
      } else {
         auto* form_wrapper = wrapper_from_stack<wrappers::form>(L, pos);
         if (!form_wrapper)
            return std::unexpected("form or string expected");
         if (!form_wrapper->stub || !dovah::form_type_is_reference(form_wrapper->stub->form_type))
            return std::unexpected("the passed-in form is not a ref");
         return run_on_params{
            .type   = dovah::conditions::run_on_type::reference,
            .entity = form_wrapper->stub
         };
      }
      // 
      // TODO: in the future, handle package data and event data
      //
   }
}

namespace {
   wrapped_type* _unwrap(wrapper& w) {
      return cls::unwrap(w);
   }
   context_type _context_of(wrapper& w) {
      return cls::context_of(w);
   }

   namespace _methods {
      template<bool TreatNilAsUnchanged>
      int _apply_table(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         if (!cls::is_not_locked(self))
            cobb::lua::error(L, "this is a locked condition on a topic info; it cannot be edited");

         constexpr const int table_pos = 2;

         switch (lua_type(L, table_pos)) {
            case LUA_TTABLE:
            case LUA_TUSERDATA:
               break;
            default:
               cobb::lua::argerror(L, table_pos, "table or userdata expected");
         }

         bool reset_params_if_invalid = false;
         bool reset_params_always     = false;

         working_type working(*wrapped);
         context_type context = cls::context_of(self);

         auto _handle_field = [L, table_pos, &working, &context](std::string_view field_name, auto pull, auto apply) {
            using pull_function_type   = decltype(pull);
            using pulled_expected_type = typename cobb::function_traits<pull_function_type>::return_type;

            lua_getfield(L, table_pos, field_name.data());

            if constexpr (TreatNilAsUnchanged) {
               if (lua_isnoneornil(L, -1)) {
                  lua_pop(L, 1);
                  return;
               }
            }

            // Construct the expected with an empty error object, since we're using std::string_view as 
            // our error object. This will avoid default-constructing the value-type just to immediately 
            // clobber it with whatever the `pull` function produces.
            pulled_expected_type result{ std::unexpect, std::string_view{} };
            if constexpr (std::is_invocable_v<pull_function_type, lua_State*, int, context_type>) {
               result = pull(L, -1, context);
            } else {
               result = pull(L, -1);
            }

            lua_pop(L, 1);

            if (!result.has_value()) {
               auto error = std::format("problem with field `{}`: {}", field_name, result.error());
               cobb::lua::argerror(L, table_pos, error.c_str());
            }
            apply(working, result.value());
         };
         auto _handle_field_with_default = [L, table_pos, &working, &context](std::string_view field_name, auto pull, auto apply, auto dv) {
            using pull_function_type   = decltype(pull);
            using pulled_expected_type = cobb::function_traits<pull_function_type>::return_type;

            lua_getfield(L, table_pos, field_name.data());
            if (lua_isnoneornil(L, -1)) {
               lua_pop(L, 1);
               if constexpr (!TreatNilAsUnchanged) {
                  apply(working, dv);
               }
               return;
            }

            // Construct the expected with an empty error object, since we're using std::string_view as 
            // our error object. This will avoid default-constructing the value-type just to immediately 
            // clobber it with whatever the `pull` function produces.
            pulled_expected_type result{ std::unexpect, std::string_view{} };
            if constexpr (std::is_invocable_v<pull_function_type, lua_State*, int, context_type>) {
               result = pull(L, -1, context);
            } else {
               result = pull(L, -1);
            }

            lua_pop(L, 1);

            if (!result.has_value()) {
               auto error = std::format("problem with field `{}`: {}", field_name, result.error());
               cobb::lua::argerror(L, table_pos, error.c_str());
            }
            apply(working, result.value());
         };

         //

         auto _pull_boolean = [](lua_State* L, int pos) -> std::expected<bool, std::string_view> {
            if (!lua_isboolean(L, pos) && !lua_isnoneornil(L, pos))
               return std::unexpected("boolean expected");
            return lua_toboolean(L, pos);
         };

         //

         _handle_field(
            "comparison",
            &pull_condition_comparison_from_lua,
            [](auto& working, auto&& v) {
               working.comparison = v;
            }
         );
         _handle_field(
            "function_name",
            &function_id_from_lua,
            [&reset_params_always](auto& working, auto&& v) {
               working.function = v;
               reset_params_always = true;
            }
         );
         _handle_field(
            "is_or_linked",
            _pull_boolean,
            [](auto& working, auto&& v) {
               working.flags.or_linked = v;
            }
         );
         _handle_field_with_default(
            "override_types_with",
            &parameter_type_override_from_lua,
            [&reset_params_if_invalid](auto& working, auto&& v) {
               working.override_types_with = v;
               reset_params_if_invalid = true;
            },
            parameter_type_override::none
         );
         {
            lua_getfield(L, table_pos, "parameters");
            if (lua_isnoneornil(L, -1)) {
               lua_pop(L, 1);
               if constexpr (!TreatNilAsUnchanged) {
                  reset_params_always = true;
               }
            } else {
               reset_params_always     = false;
               reset_params_if_invalid = false;
               //
               // We've already handled `function_name` and `override_types_with`, so we can 
               // actually set the parameters and check their validity right now.
               //
               auto result = pull_condition_parameter_set_from_lua(L, -1, self, working);
               lua_pop(L, 1);
               if (result.has_value()) {
                  auto& p_set = result.value();
                  working.event_parameters = p_set.event_data;
                  if (p_set.event_data.has_value()) {
                     working.reset_parameters();
                  } else {
                     working.parameters = std::move(p_set.parameters);
                  }
               } else {
                  std::string_view function_name = "?";
                  {
                     auto* info = dovah::conditions::function_info_by_id(working.function);
                     if (info)
                        function_name = info->name;
                  }
                  auto error = std::format("problem with field `parameters` (given condition function `{}`): {}",
                     function_name,
                     result.error()
                  );
                  cobb::lua::argerror(L, table_pos, error.c_str());
               }
            }
         }
         _handle_field(
            "run_on",
            &run_on_from_lua,
            [&reset_params_always](auto& working, auto&& v) {
               working.run_on = v;
            }
         );
         _handle_field(
            "swap_subject_and_target",
            _pull_boolean,
            [](auto& working, auto&& v) {
               working.flags.swap_subject_and_target = v;
            }
         );

         if (reset_params_always) {
            working.reset_parameters();

            working.event_parameters = {};
            //
            const auto* info = dovah::conditions::function_info_by_id(working.function);
            if (info && info->uses_event_data)
               working.event_parameters.emplace();
         } else if (reset_params_if_invalid) {
            for (size_t i = 0; i < working.parameters.size(); ++i)
               if (!working.is_parameter_valid(i))
                  working.reset_parameter(i);
         }

         if (!working.valid()) {
            cobb::lua::argerror(L, table_pos, "invalid value");
         }
         self.before_edit();
         wrapped->commit(*form, working);
         self.after_edit();
         return 0;
      }
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
      int is_or_linked(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         lua_pushboolean(L, wrapped->test_flags(wrapped_type::flag::or_linked));
         return 1;
      }
      int override_types_with(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         if (wrapped->test_flags(wrapped_type::flag::use_aliases)) {
            lua_pushstring(L, "alias");
         } else if (wrapped->test_flags(wrapped_type::flag::use_package_data)) {
            lua_pushstring(L, "packdata");
         } else {
            lua_pushstring(L, "none");
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
   }
   namespace _setters {
      template<typename CheckFunctor, typename EditFunctor>
      void _try_edit_condition(lua_State* L, CheckFunctor&& check, EditFunctor&& f) {
         core::subsystems::permissions::verify_form_write_permissions();
         
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* form    = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");
         if (!cls::is_not_locked(self))
            cobb::lua::error(L, "this is a locked condition on a topic info; it cannot be edited");

         if constexpr (std::is_invocable_v<CheckFunctor, wrapper&, working_type&>) {
            working_type working(*wrapped);
            check(self, working);
            self.before_edit();
            {
               f(working);
               wrapped->commit(*form, working);
            }
            self.after_edit();
         } else {
            if constexpr (std::is_invocable_v<CheckFunctor, wrapper&>) {
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
      }
      
      int comparison(lua_State* L) {
         working_comparison v;
         _try_edit_condition(
            L,
            [L, &v]() {
               auto result = pull_condition_comparison_from_lua(L, 2);
               if (result.has_value())
                  v = result.value();
               else
                  cobb::lua::argerror(L, 2, result.error().data());
            },
            [L, &v](working_type& working) {
               working.comparison = v;
            }
         );
         return 0;
      }
      int function(lua_State* L) {
         uint16_t function_id = 0;
         _try_edit_condition(
            L,
            [L, &function_id]() {
               auto result = function_id_from_lua(L, 2);
               if (result.has_value())
                  function_id = result.value();
               else
                  cobb::lua::argerror(L, 2, result.error().data());
            },
            [L, &function_id](working_type& working) {
               if (working.function == function_id)
                  return;
               working.function = function_id;
               working.reset_parameters();

               working.event_parameters = {};
               //
               const auto* info = dovah::conditions::function_info_by_id(function_id);
               if (info && info->uses_event_data)
                  working.event_parameters.emplace();
            }
         );
         return 0;
      }
      int is_or_linked(lua_State* L) {
         _try_edit_condition(
            L,
            [L]() {
               cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
            },
            [L](working_type& working) {
               working.flags.or_linked = lua_toboolean(L, 2);
            }
         );
         return 0;
      }
      int override_types_with(lua_State* L) {
         parameter_type_override type = parameter_type_override::none;
         _try_edit_condition(
            L,
            [L, &type]() {
               auto result = parameter_type_override_from_lua(L, 2);
               if (result.has_value())
                  type = result.value();
               else
                  cobb::lua::argerror(L, 2, result.error().data());
            },
            [L, &type](working_type& working) {
               working.override_types_with = type;
               for (size_t i = 0; i < working.parameters.size(); ++i)
                  if (!working.is_parameter_valid(i))
                     working.reset_parameter(i);
            }
         );
         return 0;
      }
      int parameters(lua_State* L) {
         api_helpers::working_condition_parameter_set v;
         _try_edit_condition(
            L,
            [L, &v](wrapper& self, working_type& working) {
               auto result = pull_condition_parameter_set_from_lua(L, 2, self, working);
               if (result.has_value())
                  v = result.value();
               else
                  cobb::lua::argerror(L, 2, result.error().data());
            },
            [L, &v](working_type& working) {
               working.event_parameters = v.event_data;
               if (v.event_data.has_value()) {
                  working.reset_parameters();
               } else {
                  working.parameters = std::move(v.parameters);
               }
            }
         );
         return 0;
      }
      int run_on(lua_State* L) {
         working_run_on_params params;
         _try_edit_condition(
            L,
            [L, &params](wrapper& self) {
               auto result = run_on_from_lua(L, 2, cls::context_of(self));
               if (result.has_value())
                  params = std::move(result.value());
               else
                  cobb::lua::argerror(L, 2, result.error().data());
            },
            [L, &params](working_type& working) {
               working.run_on = params;
            }
         );
         return 0;
      }
      int swap_subject_and_target(lua_State* L) {
         _try_edit_condition(
            L,
            [L]() {
               cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
            },
            [L](working_type& working) {
               working.flags.swap_subject_and_target = lua_toboolean(L, 2);
            }
         );
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "assign",         &_methods::_apply_table<true> },
      { "overwrite_with", &_methods::_apply_table<false> },
   };
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "comparison",              &_getters::comparison },
      { "function_name",           &_getters::function },
      { "is_or_linked",            &_getters::is_or_linked },
      { "override_types_with",     &_getters::override_types_with },
      { "owning_package",          &_getters::owning_package },
      { "owning_quest",            &_getters::owning_quest },
      { "parameters",              &_getters::parameters },
      { "run_on",                  &_getters::run_on },
      { "swap_subject_and_target", &_getters::swap_subject_and_target },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "comparison",              &_setters::comparison },
      { "function_name",           &_setters::function },
      { "is_or_linked",            &_setters::is_or_linked },
      { "override_types_with",     &_setters::override_types_with },
      { "parameters",              &_setters::parameters },
      { "run_on",                  &_setters::run_on },
      { "swap_subject_and_target", &_setters::swap_subject_and_target },
   };
}