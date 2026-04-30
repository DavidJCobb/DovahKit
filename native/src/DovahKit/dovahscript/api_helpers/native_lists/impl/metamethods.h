#pragma once
struct lua_State;

namespace dovahscript::api_helpers::native_lists::impl::metamethods {
   extern int __index(lua_State* L);
   extern int __newindex(lua_State* L);
   extern int __ipairs(lua_State* L);

   extern void prepare_iterator_metatables(lua_State* L);
}