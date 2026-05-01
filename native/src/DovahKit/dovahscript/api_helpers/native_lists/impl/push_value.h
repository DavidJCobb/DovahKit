#pragma once
#include <concepts>
#include <string>
#include <string_view>
#include "lua.h"
#include "dovah/form_reference_t.h"
#include "dovahscript/push_native_object.h"

namespace dovahscript::api_helpers::native_lists::impl {
   inline int default_push_value(lua_State* L, const dovah::form_reference_t& v) {
      return push_native_object(v.get_form_stub());
   }
   inline int default_push_value(lua_State* L, dovah::form_stub* v) { // it's rare to expose unmanaged pointers, but sometimes we do
      return push_native_object(v);
   }
   inline int default_push_value(lua_State* L, bool v) {
      lua_pushboolean(L, v);
      return 1;
   }
   inline int default_push_value(lua_State* L, lua_Integer v) {
      lua_pushinteger(L, v);
      return 1;
   }
   inline int default_push_value(lua_State* L, lua_Number v) {
      lua_pushnumber(L, v);
      return 1;
   }
   inline int default_push_value(lua_State* L, const std::string& v) {
      lua_pushlstring(L, v.data(), v.size());
      return 1;
   }
   inline int default_push_value(lua_State* L, const std::string_view& v) {
      lua_pushlstring(L, v.data(), v.size());
      return 1;
   }

   template<typename Spec>
   concept value_type_is_default_pushable = requires {
      typename Spec::value_stored_type;
      requires requires(lua_State* L, const typename Spec::value_stored_type& v) {
         { default_push_value(L, v) } -> std::same_as<int>;
      };
   };
}