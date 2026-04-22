#include "./push_pull_comparison_operand.h"
#include "lua.h"
#include "dovah/form_stub.h"
#include "dovah/form_types.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"
#include "dovahscript/wrappers/form/form.h"

namespace dovahscript::api_helpers::conditions {
   extern std::expected<decltype(working_comparison::operand), std::string_view> pull_comparison_operand(lua_State* L, int pos) {
      if (lua_isnumber(L, pos)) {
         return (float)lua_tonumber(L, pos);
      }
      auto* other = wrapper_from_stack<wrappers::form>(L, pos);
      if (!other || !other->stub || other->stub->form_type != dovah::form_type::global)
         return std::unexpected("operand must be a number or a form of type `global`");
      return other->stub;
   }

   extern void push_comparison_operand(lua_State* L, const decltype(condition_type::comparison_data::operand)& operand) {
      if (std::holds_alternative<float>(operand)) {
         lua_pushnumber(L, std::get<float>(operand));
         return;
      }
      if (std::holds_alternative<dovah::form_reference_t>(operand)) {
         int c = push_native_object(std::get<dovah::form_reference_t>(operand));
         if (c > 0) {
            if (c > 1)
               lua_pop(L, c - 1);
            return;
         }
      }
      lua_pushnil(L);
   }
}