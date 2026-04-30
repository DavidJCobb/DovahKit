#pragma once
#include <string_view>
#include "lua.h"
#include "dovahscript/wrapper.h"
#include "./member_function_spec.h"
#include "./impl/metamethods.h"

#include "./clear.h"
#include "./get_collection_length.h"
#include "./get_item_by_index.h"
#include "./insert.h"
#include "./remove_item_at_index.h"
#include "./set_item_at_index.h"

namespace dovahscript::api_helpers::native_lists {
   template<
      const std::string_view& CollectionMetatableKey,
      const std::string_view& ClassName,
      typename Spec
   >
      requires impl::is_fully_valid_spec<Spec>
   void define_metatable(lua_State* L) {
      impl::metamethods::prepare_iterator_metatables(L);
      
      luaL_newmetatable(L, CollectionMetatableKey.data()); // STACK: [newmeta]
      auto index_mt = lua_gettop(L);
      
      lua_createtable(L, 1, 0);  // rtti = {}
      lua_pushvalue  (L, -2);    //
      lua_rawseti    (L, -2, 1); // rtti[1] = meta
      lua_setfield(L, -2, "__classlist"); // meta.__classlist = rtti
      lua_pushlstring(L, ClassName.data(), ClassName.size());
      lua_setfield(L, -2, "__name");
      
      lua_pushcfunction(L, &impl::metamethods::__index);
      lua_setfield(L, index_mt, "__index");
      lua_pushcfunction(L, &impl::metamethods::__newindex);
      lua_setfield(L, index_mt, "__newindex");
      lua_pushcfunction(L, &impl::metamethods::__ipairs);
      lua_setfield(L, index_mt, "__pairs");
      lua_pushcfunction(L, &impl::metamethods::__ipairs);
      lua_setfield(L, index_mt, "__ipairs");
      
      lua_pushcfunction(L, &wrapper::__gc);
      lua_setfield(L, index_mt, "__gc");
      {
         lua_CFunction f;
         if constexpr (impl::get_collection_length::valid<Spec>) {
            f = &Spec::get_collection_length;
         } else {
            static_assert(impl::get_collection_length::defaultable<Spec>);
            f = &get_collection_length<Spec>;
         }
         lua_pushcfunction(L, f);
         lua_setfield(L, index_mt, "__len");
      }
      {
         lua_CFunction f;
         if constexpr (impl::get_item_by_index::valid<Spec>) {
            f = &Spec::get_item_by_index;
         } else {
            static_assert(impl::get_item_by_index::defaultable<Spec>);
            f = &get_item_by_index<Spec>;
         }
         lua_pushcfunction(L, f);
         lua_setfield(L, index_mt, "lookup_item_by_index");
      }
      if constexpr (impl::pull_value::valid<Spec>) {
         lua_pushcfunction(L, &set_item_at_index<Spec>);
         lua_setfield(L, index_mt, "set_item");
      }

      if constexpr (impl::pull_value::valid<Spec> || Spec::allow_removals) {
         lua_createtable(L, 0, 3);
         if constexpr (Spec::allow_removals) {
            lua_pushcfunction(L, &clear<Spec>);
            lua_setfield(L, -2, "clear");
         }
         if constexpr (impl::pull_value::valid<Spec>) {
            lua_pushcfunction(L, &insert<Spec>);
            lua_setfield(L, -2, "insert");
         }
         if constexpr (Spec::allow_removals) {
            lua_pushcfunction(L, &remove_item_at_index<Spec>);
            lua_setfield(L, -2, "remove");
         }
         lua_setfield(L, index_mt, "members");
      }
      
      lua_pop(L, 1); // pop metatable
   }
}