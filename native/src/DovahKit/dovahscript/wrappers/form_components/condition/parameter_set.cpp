#include "./parameter_set.h"
#include <limits>
#include "helpers/lua/error.h"
#include "helpers/lua/warning.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/data/conditions/all_function_info.h"
#include "dovah/data/conditions/event_function.h"
#include "dovah/data/conditions/function_info.h"
#include "dovah/data/conditions/parameter_typeinfo.h"
#include "dovah/forms/components/conditions.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "dovah/forms/Quest.h"
#include "../condition.h"
#include "./impl/pull_condition_parameter_from_lua.h"

#include "../../form/quest/alias.h"

#define FOR_EACH_EVENT_FUNCTION_ID(DO) \
   DO(GetIsID) \
   DO(IsInList) \
   DO(GetValue) \
   DO(HasKeyword) \
   DO(GetItemValue) \

namespace {
   using namespace dovahscript;
   using cls          = wrappers::condition_parameter_set;
   using working_type = dovah::loaded_forms::components::conditions::working_condition;

   bool _condition_uses_event_params(const wrappers::condition::wrapped_type& data) {
      const auto* func = data.get_function();
      if (!func)
         return false;
      return func->uses_event_data;
   }

   namespace _getters {
      int _parameter_by_zero_based_index(lua_State* L, size_t which) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_parameter_set wrapper has no underlying object (deleted?)");

         auto underlying = data->get_argument_underlying_type(which);
         auto param      = data->get_parameter(which);
         if (std::holds_alternative<std::monostate>(param)) {
            lua_pushnil(L);
         } else if (std::holds_alternative<uint32_t>(param)) {
            switch (underlying) {
               case dovah::conditions::parameter_underlying_type::alias:
                  if (auto* quest = wrappers::condition::context_of(self).quest) {
                     return wrappers::quest_alias::wrap(L, quest, std::get<uint32_t>(param));
                  }
                  break;
               case dovah::conditions::parameter_underlying_type::int_unsigned:
               case dovah::conditions::parameter_underlying_type::quest_stage:
               default:
                  lua_pushinteger(L, std::get<uint32_t>(param));
                  break;
               case dovah::conditions::parameter_underlying_type::package_data:
                  lua_pushinteger(L, std::get<uint32_t>(param)); // TODO: return a wrapped package data
                  break;
            }
         } else if (std::holds_alternative<char>(param)) {
            lua_pushlstring(L, &std::get<char>(param), 1);
         } else if (std::holds_alternative<float>(param)) {
            lua_pushnumber(L, std::get<float>(param));
         } else if (std::holds_alternative<int32_t>(param)) {
            auto v = std::get<int32_t>(param);
            if (underlying == dovah::conditions::parameter_underlying_type::enumeration) {
               const auto* typeinfo = data->get_argument_type(which);
               if (typeinfo && typeinfo->enumeration_info.has_value()) {
                  const auto& enumeration = typeinfo->enumeration_info.value();
                  for (size_t i = 0; i < enumeration.size; ++i) {
                     const auto& member = enumeration.members[i];
                     if (member.value == v) {
                        lua_pushstring(L, member.name.data());
                        return 1;
                     }
                  }
               }
            }
            lua_pushinteger(L, v);
         } else if (std::holds_alternative<dovah::form_stub*>(param)) {
            return push_native_object(std::get<dovah::form_stub*>(param));
         } else if (std::holds_alternative<std::string>(param)) {
            auto& data = std::get<std::string>(param);
            lua_pushstring(L, data.c_str());
         } else {
            return 0;
         }
         return 1;
      }

      int event_function(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_parameter_set wrapper has no underlying object (deleted?)");
         if (!_condition_uses_event_params(*data))
            return 0;

         auto& params = data->get_event_parameters();
         switch (params.function) {
            #define CASE(name, ...) case dovah::conditions::event_function::name: lua_pushstring(L, #name); return 1;
            FOR_EACH_EVENT_FUNCTION_ID(CASE)
            #undef CASE
         }
         return 0;
      }
      int event_member(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_parameter_set wrapper has no underlying object (deleted?)");
         if (!_condition_uses_event_params(*data))
            return 0;

         auto& params = data->get_event_parameters();
         if (params.member == 0)
            return 0;
         //
         // Event members are defined as two-CCs.
         //
         char name[3] = { '\0', '\0', '\0' };
         if constexpr (std::endian::native == std::endian::little) {
            name[0] = params.member >> 8;
            name[1] = params.member;
         } else {
            name[0] = params.member;
            name[1] = params.member >> 8;
         }
         lua_pushlstring(L, name, 2);
         return 1;
      }
      int event_form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_parameter_set wrapper has no underlying object (deleted?)");
         if (!_condition_uses_event_params(*data))
            return 0;

         auto& params = data->get_event_parameters();
         return push_native_object(params.form);
      }
      
      int parameter_1(lua_State* L) {
         return _parameter_by_zero_based_index(L, 0);
      }
      int parameter_2(lua_State* L) {
         return _parameter_by_zero_based_index(L, 1);
      }
   }
   namespace _setters {
      void _warn_on_param(
         lua_State* L,
         size_t which,
         uint16_t function_id,
         const dovah::loaded_forms::components::conditions::working_parameter& param
      ) {
         if (which == 0) {
            switch (function_id) {
               case dovah::conditions::function_id_by_name("GetPos"):
                  if (auto* casted = std::get_if<char>(&param)) {
                     switch (*casted) {
                        case 'X':
                        case 'Y':
                        case 'Z':
                           break;
                        case 'x':
                        case 'y':
                        case 'z':
                           cobb::lua::warning(L, "axis '%c' is not valid for GetPos conditions (axis name must be uppercase); this condition will always return false in-game", *casted);
                           break;
                        default:
                           cobb::lua::warning(L, "axis '%c' is not valid for GetPos conditions (axis name must be 'X', 'Y', or 'Z'); this condition will always return false in - game", *casted);
                           break;
                     }
                  }
                  break;
            }
         } else {
            // ...
         }
      }

      void _edit_param(lua_State* L, size_t which) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_parameter_set wrapper has no underlying object (deleted?)");
         if (!wrappers::condition::is_not_locked(self))
            cobb::lua::error(L, "this is a locked condition on a topic info; it cannot be edited");

         working_type working(*data);
         {
            auto result = pull_condition_parameter_from_lua(L, 2, self, working, which);
            if (result.has_value()) {
               auto& param = result.value();
               _warn_on_param(L, which, working.function, param);
               working.parameters[which] = param;
            } else {
               cobb::lua::argerror(L, 2, result.error().data());
            }
         }
         if (!working.valid()) { // just in case
            cobb::lua::argerror(L, 2, "invalid value");
         }

         self.before_edit();
         data->commit(*form, working);
         self.after_edit();
      }

      template<typename CheckFunctor, typename EditFunctor>
      void _edit_event_params(lua_State* L, CheckFunctor&& check, EditFunctor&& edit) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_parameter_set wrapper has no underlying object (deleted?)");
         if (!wrappers::condition::is_not_locked(self))
            cobb::lua::error(L, "this is a locked condition on a topic info; it cannot be edited");
         if (!_condition_uses_event_params(*data))
            cobb::lua::error(L, "this property is only permitted for condition functions that use event data");

         check();

         self.before_edit();
         {
            working_type working(*data);

            auto& opt = working.event_parameters;
            if (!opt.has_value())
               opt.emplace();
            edit(opt.value());

            data->commit(*form, working);
         }
         self.after_edit();
      }

      int event_function(lua_State* L) {
         dovah::conditions::event_function::type fn;
         _edit_event_params(
            L,
            [L, &fn]() {
               auto result = pull_condition_event_function_from_lua(L, 2);
               if (result.has_value())
                  fn = result.value();
               else
                  cobb::lua::argerror(L, 2, result.error().data());
            },
            [&fn](auto& params) {
               params.function = fn;
            }
         );
         return 0;
      }
      int event_member(lua_State* L) {
         uint16_t member = 0;
         _edit_event_params(
            L,
            [L, &member]() {
               auto result = pull_condition_event_member_from_lua(L, 2);
               if (result.has_value())
                  member = result.value();
               else
                  cobb::lua::argerror(L, 2, result.error().data());
            },
            [&member](auto& params) {
               params.member = member;
            }
         );
         return 0;
      }
      int event_form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);

         dovah::form_stub* stub = nullptr;
         _edit_event_params(
            L,
            [L, &stub]() {
               auto result = pull_condition_event_form_from_lua(L, 2);
               if (result.has_value())
                  stub = result.value();
               else
                  cobb::lua::argerror(L, 2, result.error().data());
            },
            [&stub](working_type::event_data& params) {
               params.form = stub;
            }
         );
         return 0;
      }
      
      int parameter_1(lua_State* L) {
         _edit_param(L, 0);
         return 0;
      }
      int parameter_2(lua_State* L) {
         _edit_param(L, 1);
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "function", &_getters::event_function },
      { "member",   &_getters::event_member },
      { "form",     &_getters::event_form },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "function", &_setters::event_function },
      { "member",   &_setters::event_member },
      { "form",     &_setters::event_form },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      //
      // For 99% of conditions, there are at most two parameters, identified by 
      // index. Ordinarily, a collection would be the most appropriate way to 
      // grant access to them. However, an extremely small number of condition 
      // functions instead use event data and, thus, event parameters, which are 
      // structured differently and ideally referred to by specific identifiers.
      // 
      // Okay, so use a collection that allows named elements. What's the problem? 
      // Well, those collections work by mapping names to indices, and that... is 
      // not really appropriate here. I'm not sure I want the event function, 
      // member, and form to be regarded as synonymous with parameters 1, 2, and 3, 
      // especially when every other condition only has 2 parameters.
      // 
      // I mean, maybe I do? But if so, we can switch to using a collection later.
      // 
      // Anywho: because we're not using a collection, we need to define getters 
      // and setters for the `1` and `2` table keys. In Lua, however, foo[1] is 
      // not the same as foo["1"], and our machinery for creating [gs]etters in 
      // bulk uses string keys. So, we have to do this manually.
      //
      lua_pushcfunction(L, &_getters::parameter_1);
      lua_rawseti(L, -3, 1);
      lua_pushcfunction(L, &_getters::parameter_2);
      lua_rawseti(L, -3, 2);

      lua_pushcfunction(L, &_setters::parameter_1);
      lua_rawseti(L, -2, 1);
      lua_pushcfunction(L, &_setters::parameter_2);
      lua_rawseti(L, -2, 2);
   }
}