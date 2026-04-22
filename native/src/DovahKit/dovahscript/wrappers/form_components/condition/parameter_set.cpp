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

#include "dovahscript/api_helpers/conditions/push_pull_event_parameter.h"
#include "dovahscript/api_helpers/conditions/push_pull_indexed_parameter.h"

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
         api_helpers::conditions::push_indexed_parameter(
            L,
            wrappers::condition::context_of(self),
            data->get_argument_type(which),
            data->get_argument_underlying_type(which),
            data->get_parameter(which)
         );
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
         api_helpers::conditions::push_event_function(L, params.function);
         return 1;
      }
      int event_member(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_parameter_set wrapper has no underlying object (deleted?)");
         if (!_condition_uses_event_params(*data))
            return 0;

         auto& params = data->get_event_parameters();
         api_helpers::conditions::push_event_member(L, params.member);
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
         api_helpers::conditions::push_event_form(L, params.form.get_form_stub());
         return 1;
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
            auto result = api_helpers::conditions::pull_indexed_parameter(
               L,
               2,
               wrappers::condition::context_of(self),
               working.get_argument_underlying_type(which),
               working.get_effective_argument_typeinfo(which)
            );
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
               auto result = api_helpers::conditions::pull_event_function(L, 2);
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
               auto result = api_helpers::conditions::pull_event_member(L, 2);
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
               auto result = api_helpers::conditions::pull_event_form(L, 2);
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