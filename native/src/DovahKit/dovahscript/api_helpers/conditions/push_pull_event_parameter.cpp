#include "./push_pull_event_parameter.h"
#include <bit>
#include "lua.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"
#include "dovahscript/wrappers/form/form.h"

#define FOR_EACH_EVENT_FUNCTION_ID(DO) \
   DO(GetIsID) \
   DO(IsInList) \
   DO(GetValue) \
   DO(HasKeyword) \
   DO(GetItemValue) \

namespace dovahscript::api_helpers::conditions {
   extern std::expected<dovah::conditions::event_function::type, std::string_view> pull_event_function(lua_State* L, int pos) {
      if (!lua_isstring(L, pos))
         return std::unexpected("string expected");
      std::string_view v = lua_tostring(L, pos);
      #define CASE(name, ...) if (v == #name) return dovah::conditions::event_function::name;
      FOR_EACH_EVENT_FUNCTION_ID(CASE)
      #undef CASE
      return std::unexpected("unrecognized event function name");
   }
   extern std::expected<uint16_t, std::string_view> pull_event_member(lua_State* L, int pos) {
      if (!lua_isstring(L, pos))
         return std::unexpected("string expected");
      std::string_view name = lua_tostring(L, pos);
      if (name.size() != 2)
         return std::unexpected("argument is not an event member signature");
      if constexpr (std::endian::native == std::endian::little) { // this really should be done in the condition internals...
         return name[1] | ((uint16_t)name[0] << 8);
         // DKConditionListModel handles this too, so if we ever do fix it, we should fix it there too
      } else {
         return name[0] | ((uint16_t)name[1] << 8);
      }
   }
   extern std::expected<dovah::form_stub*, std::string_view> pull_event_form(lua_State* L, int pos) {
      if (lua_isnoneornil(L, pos))
         return nullptr;
      auto* other = wrapper_from_stack<wrappers::form>(L, pos);
      if (!other)
         return std::unexpected("form or nil expected");
      return other->stub;
   }

   extern void push_event_function(lua_State* L, uint16_t function_id) {
      switch (function_id) {
         #define CASE(name, ...) case dovah::conditions::event_function::name: lua_pushstring(L, #name); return;
         FOR_EACH_EVENT_FUNCTION_ID(CASE)
         #undef CASE
      }
      lua_pushnil(L);
   }
   extern void push_event_member(lua_State* L, uint16_t member) {
      if (member == 0) {
         lua_pushnil(L);
         return;
      }
      char name[3] = { '\0', '\0', '\0' };
      if constexpr (std::endian::native == std::endian::little) {
         name[0] = member >> 8;
         name[1] = member;
      } else {
         name[0] = member;
         name[1] = member >> 8;
      }
      lua_pushlstring(L, name, 2);
   }
   extern void push_event_form(lua_State* L, dovah::form_stub* form) {
      int c = push_native_object(form);
      if (c > 0) {
         if (c > 1)
            lua_pop(L, c - 1);
         return;
      }
      lua_pushnil(L);
   }
}