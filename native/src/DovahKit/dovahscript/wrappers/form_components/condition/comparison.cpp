#include "./comparison.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/conditions.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "../condition.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::condition_comparison;
   using working_type = dovah::loaded_forms::components::conditions::working_condition;

   namespace _getters {
      int operator_(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_comparison wrapper has no underlying object (deleted?)");
         auto& comparison = data->get_comparison();
         switch (comparison.op) {
            using enum dovah::conditions::comparison_operator;
            case equal:
               lua_pushstring(L, "==");
               return 1;
            case not_equal:
               lua_pushstring(L, "!=");
               return 1;
            case greater:
               lua_pushstring(L, ">");
               return 1;
            case greater_or_equal:
               lua_pushstring(L, ">=");
               return 1;
            case less:
               lua_pushstring(L, "<");
               return 1;
            case less_or_equal:
               lua_pushstring(L, "<=");
               return 1;
         }
         return 0;
      }
      int operand(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_comparison wrapper has no underlying object (deleted?)");
         auto& operand = data->get_comparison().operand;
         if (std::holds_alternative<float>(operand)) {
            lua_pushnumber(L, std::get<float>(operand));
         } else if (std::holds_alternative<dovah::form_reference_t>(operand)) {
            return push_native_object(std::get<dovah::form_reference_t>(operand));
         } else {
            lua_pushnil(L);
         }
         return 1;
      }
   }
   namespace _setters {
      int operator_(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_comparison wrapper has no underlying object (deleted?)");
         if (!wrappers::condition::is_not_locked(self))
            cobb::lua::error(L, "this is a locked condition on a topic info; it cannot be edited");
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");

         using enumeration = dovah::conditions::comparison_operator;

         const auto operator_str = std::string_view(lua_tostring(L, 2));
         enumeration value;
         if (operator_str == "==")
            value = enumeration::equal;
         else if (operator_str == "!=")
            value = enumeration::not_equal;
         else if (operator_str == ">")
            value = enumeration::greater;
         else if (operator_str == ">=")
            value = enumeration::greater_or_equal;
         else if (operator_str == "<")
            value = enumeration::less;
         else if (operator_str == "<=")
            value = enumeration::less_or_equal;
         else
            cobb::lua::argcheck(L, false, 2, "unrecognized operator");

         self.before_edit();
         {
            working_type working(*data);
            working.comparison.op = value;
            data->commit(*form, working);
         }
         self.after_edit();
         return 0;
      }
      int operand(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_comparison wrapper has no underlying object (deleted?)");
         if (!wrappers::condition::is_not_locked(self))
            cobb::lua::error(L, "this is a locked condition on a topic info; it cannot be edited");

         dovah::form_stub* operand_form  = nullptr;
         float             operand_float = 0.0F;
         if (lua_isnumber(L, 2)) {
            operand_float = lua_tonumber(L, 2);
         } else {
            operand_form = pull_form_stub_argument(L, 2, dovah::form_type::global);
            if (!operand_form)
               cobb::lua::argcheck(L, false, 2, "cannot compare to NONE");
         }
         
         self.before_edit();
         {
            working_type working(*data);
            auto& dst = working.comparison.operand;
            if (operand_form) {
               dst = operand_form;
            } else {
               dst.emplace<float>(operand_float);
            }
            data->commit(*form, working);
         }
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "operator", &_getters::operator_ },
      { "operand",  &_getters::operand },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "operator", &_setters::operator_ },
      { "operand",  &_setters::operand },
   };
}