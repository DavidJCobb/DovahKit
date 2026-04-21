#pragma once
#include <string>
#include <string_view>
#include "lua.h"
#include "dovahscript/push_native_object.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   namespace checks {
      inline std::string_view integer(lua_State* L, int pos) {
         if (!lua_isinteger(L, pos))
            return "integer expected";
         return {};
      }
      inline std::string_view string(lua_State* L, int pos) {
         if (!lua_isstring(L, pos))
            return "string expected";
         return {};
      }
   }
   namespace pull {
      inline dovah::form_stub* form_stub(lua_State* L, int pos) {
         auto* w = wrapper_from_stack<wrappers::form>(L, -1);
         if (!w)
            return nullptr;
         return w->stub;
      }
      inline auto integer(lua_State* L, int pos) {
         return lua_tointeger(L, pos);
      }
      inline std::string_view string_view(lua_State* L, int pos) {
         return lua_tostring(L, pos);
      }
   }
   namespace push {
      inline void form_reference(lua_State* L, const dovah::form_reference_t& value) {
         int count = push_native_object(value.get_form_stub());
         if (count == 0)
            lua_pushnil(L);
         else if (count > 1)
            lua_pop(L, count - 1);
      }
      inline void form_stub(lua_State* L, dovah::form_stub* value) {
         int count = push_native_object(value);
         if (count == 0)
            lua_pushnil(L);
         else if (count > 1)
            lua_pop(L, count - 1);
      }
      inline void string(lua_State* L, const std::string& v) {
         lua_pushstring(L, v.c_str());
      }
      inline void string_view(lua_State* L, const std::string_view& v) {
         lua_pushlstring(L, v.data(), v.size());
      }
   }
}