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
#include "dovah/forms/components/conditions.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/TopicInfo.h"

#include "./condition/comparison.h"
#include "./condition/parameter_set.h"
#include "dovahscript/api_helpers/conditions/pull_comparison_as_table.h"
#include "dovahscript/api_helpers/conditions/pull_parameters_as_table.h"
#include "dovahscript/api_helpers/conditions/push_pull_comparison_operand.h"
#include "dovahscript/api_helpers/conditions/push_pull_comparison_operator.h"
#include "dovahscript/api_helpers/conditions/push_pull_event_parameter.h"
#include "dovahscript/api_helpers/conditions/push_pull_indexed_parameter.h"
#include "dovahscript/api_helpers/conditions/push_pull_parameter_type_override.h"
#include "dovahscript/api_helpers/conditions/push_pull_run_on.h"
#include "dovahscript/api_helpers/fail_table_if_expandos.h"
#include "dovahscript/wrappers/form/form.h"

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

namespace {
   // If `true`, then attempting to overwrite a condition with a table-or-userdata will 
   // fail if the source table specifies an `owning_package` or `owning_quest` field that 
   // doesn't match the owning package/quest of the destination condition. (This behavior 
   // is skipped when overwriting a condition with another condition.)
   constexpr const bool apply_table_fails_on_mismatched_context = true;

   // If `true`, then `condition:copy_as_table()` stores the `owning_package` and the 
   // `owning_quest` fields on the output table. This probably shouldn't be set to `true` 
   // unless `apply_table_fails_on_mismatched_context` is set to `false`, as otherwise, 
   // the ergonomics will be miserable for code like the following, when the condition has 
   // an owning package or quest:
   //
   //    local t = cnd:copy_as_table()
   // 
   //    -- some hypothetical change, possibly behind multiple layers of abstraction 
   //    -- within the victim script
   //    t.is_or_linked = false
   // 
   //    cnd:overwrite_with(t)
   //
   constexpr const bool copy_as_table_also_saves_context = false;
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

         bool arg_is_another_condition = false;
         {
            auto* arg_wrap = wrapper_from_stack<cls>(L, table_pos);
            if (arg_wrap) {
               arg_is_another_condition = true;
               if (arg_wrap == &self)
                  //
                  // Early-out on self-assignment.
                  //
                  return 0;
            }
         }

         api_helpers::fail_table_if_expandos(L, table_pos, std::array<std::string_view, 9>{
            "comparison",
            "function_name",
            "is_or_linked",
            "override_types_with",
            "parameters",
            "run_on",
            "swap_subject_and_target",
            //
            // Context fields:
            //
            "owning_package",
            "owning_quest",
         });

         bool reset_params_if_invalid = false;
         bool reset_params_always     = false;

         working_type working(*wrapped);
         context_type context = cls::context_of(self);

         //
         // Check if `owning_package` or `owning_quest` are set on the argument, and if 
         // so, require that they match the destination condition.
         // 
         // NOTE: We deliberately ignore mismatches when overwriting one condition with 
         // another, to more easily allow copying conditions across forms.
         //
         if constexpr (apply_table_fails_on_mismatched_context) {
            if (!arg_is_another_condition) {
               auto _pull_form = [L](int pos) -> std::optional<dovah::form_stub*> {
                  auto* wrapper = wrapper_from_stack<wrappers::form>(L, pos);
                  if (!wrapper)
                     return {};
                  return wrapper->stub;
               };

               lua_getfield(L, table_pos, "owning_package");
               if (!lua_isnoneornil(L, -1)) {
                  auto opt_form = _pull_form(-1);
                  if (!opt_form.has_value() || opt_form.value() != context.package)
                     cobb::lua::argerror(L, 2, "the table-or-userdata specified an `owning_package` that is not this condition's owning package (you don't need to specify the owning form, so consider not doing that instead)");
               }
               lua_pop(L, 1);
               lua_getfield(L, table_pos, "owning_quest");
               if (!lua_isnoneornil(L, -1)) {
                  auto opt_form = _pull_form(-1);
                  if (!opt_form.has_value() || opt_form.value() != context.quest)
                     cobb::lua::argerror(L, 2, "the table-or-userdata specified an `owning_quest` that is not this condition's owning quest (you don't need to specify the owning form, so consider not doing that instead)");
               }
               lua_pop(L, 1);
            }
         }

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
            &api_helpers::conditions::pull_comparison_as_table,
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
            &api_helpers::conditions::pull_parameter_type_override,
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
               auto result = api_helpers::conditions::pull_parameters_as_table(
                  L,
                  -1,
                  *dovah::conditions::function_info_by_id(working.function),
                  context,
                  working.override_types_with
               );
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
            &api_helpers::conditions::pull_run_on,
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

      int copy_as_table(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = _unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "condition wrapper has no underlying object (deleted?)");

         const auto* function_info = wrapped->get_function();

         lua_createtable(L, 0, 7);
         {
            auto& src_cmp = wrapped->get_comparison();
            lua_createtable(L, 0, 2);
            {
               api_helpers::conditions::push_comparison_operator(L, src_cmp.op);
               lua_setfield(L, -2, "operator");
               api_helpers::conditions::push_comparison_operand(L, src_cmp.operand);
               lua_setfield(L, -2, "operand");
            }
            lua_setfield(L, -2, "comparison");
         }
         {
            if (function_info) {
               lua_pushlstring(L, function_info->name.data(), function_info->name.size());
            } else {
               lua_pushnil(L);
            }
            lua_setfield(L, -2, "function_name");
         }
         {
            lua_pushboolean(L, wrapped->test_flags(wrapped_type::flag::or_linked));
            lua_setfield(L, -2, "is_or_linked");
         }
         {
            api_helpers::conditions::push_parameter_type_override(L, *wrapped);
            lua_setfield(L, -2, "override_types_with");
         }
         {
            lua_createtable(L, 0, 5);
            if (function_info) {
               if (function_info->uses_event_data) {
                  const auto& ep = wrapped->get_event_parameters();
                  api_helpers::conditions::push_event_function(L, ep.function);
                  lua_setfield(L, -2, "function");
                  api_helpers::conditions::push_event_member(L, ep.member);
                  lua_setfield(L, -2, "member");
                  api_helpers::conditions::push_event_form(L, ep.form.get_form_stub());
                  lua_setfield(L, -2, "form");
               } else {
                  for (size_t i = 0; i < 1; ++i) {
                     api_helpers::conditions::push_indexed_parameter(
                        L,
                        wrappers::condition::context_of(self),
                        wrapped->get_argument_type(i),
                        wrapped->get_argument_underlying_type(i),
                        wrapped->get_parameter(i)
                     );
                     lua_seti(L, -2, i + 1);
                  }
               }
            }
            lua_setfield(L, -2, "parameters");
         }
         {
            api_helpers::conditions::push_run_on(L, wrapped->get_run_on_data(), cls::context_of(self));
            lua_setfield(L, -2, "run_on");
         }
         {
            lua_pushboolean(L, wrapped->test_flags(wrapped_type::flag::swap_subject_and_target));
            lua_setfield(L, -2, "swap_subject_and_target");
         }

         if constexpr (copy_as_table_also_saves_context) {
            auto context = cls::context_of(self);
            int  pushed  = push_native_object(context.package);
            if (pushed > 0) {
               if (pushed > 1)
                  lua_pop(L, pushed - 1);
               lua_setfield(L, -2, "owning_package");
            }
            pushed = push_native_object(context.quest);
            if (pushed > 0) {
               if (pushed > 1)
                  lua_pop(L, pushed - 1);
               lua_setfield(L, -2, "owning_quest");
            }
         }

         return 1;
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
         api_helpers::conditions::push_parameter_type_override(L, *wrapped);
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
         api_helpers::conditions::push_run_on(L, wrapped->get_run_on_data(), cls::context_of(self));
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
               auto result = api_helpers::conditions::pull_comparison_as_table(L, 2);
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
               auto result = api_helpers::conditions::pull_parameter_type_override(L, 2);
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
         api_helpers::conditions::working_parameter_set v;
         _try_edit_condition(
            L,
            [L, &v](wrapper& self, working_type& working) {
               const auto* function_info = dovah::conditions::function_info_by_id(working.function);
               if (!function_info)
                  cobb::lua::error(L, "internal error: unable to find function info for this condition");
               auto result = api_helpers::conditions::pull_parameters_as_table(L, 2, *function_info, cls::context_of(self), working.override_types_with);
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
               auto result = api_helpers::conditions::pull_run_on(L, 2, cls::context_of(self));
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
      { "copy_as_table",  &_methods::copy_as_table },
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