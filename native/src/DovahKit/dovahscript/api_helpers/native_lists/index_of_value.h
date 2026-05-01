#pragma once
#include "helpers/lua/warning.h"
#include "lua.h"
#include "dovahscript/wrapper.h"
#include "dovahscript/wrappers/base.h"
#include "./impl/concepts/is_fully_valid_spec.h"
#include "./impl/concepts/values_are_subobjects.h"
#include "./impl/concepts/storage_is_bifurcated.h"
#include "./impl/fields/get_collection_length.h"
#include "./impl/fields/pull_value.h"
#include "./impl/fields/validate_value.h"
#include "./get_item_by_index.h"

namespace dovahscript::api_helpers::native_lists {
   namespace impl {
      inline std::optional<size_t> get_item_wrapper_index(lua_State* L, int pos) {
         auto* w = wrapper_from_stack<wrapper_metatable>(L, pos);
         if (!w)
            return {};
         if (!w->depth || w->is_collection)
            return {};
         auto& lp = w->last_part();
         if (lp.noncontiguous || !lp.signature)
            return 0;
         return lp.index;
      }

      namespace index_of_value {
         template<typename Spec>
            requires (
               impl::concepts::is_fully_valid_spec<Spec>
            && impl::fields::get_collection_length::defaultable<Spec>
            && impl::fields::pull_value::valid<Spec>
            )
         int for_direct(lua_State* L) {
            using value_working_type = typename Spec::value_working_type;

            auto& self = Spec::pull_collection(L);
            
            value_working_type v = Spec::pull_value(L, 2);
            if constexpr (impl::concepts::storage_is_bifurcated<Spec>) {
               const auto pair = Spec::unwrap_collection(self);
               size_t offset = 0;
               if (pair.first) {
                  const auto& list = *pair.first;
                  for (size_t i = 0; i < list.size(); ++i) {
                     if (list[i] == v) {
                        lua_pushinteger(L, i + 1);
                        return 1;
                     }
                  }
                  offset = list.size();
               }
               if (pair.second) {
                  const auto& list = *pair.second;
                  for (size_t i = 0; i < list.size(); ++i) {
                     if (list[i] == v) {
                        lua_pushinteger(L, i + offset + 1);
                        return 1;
                     }
                  }
               }
            } else {
               const auto* list_ptr = Spec::unwrap_collection(self);
               if (!list_ptr)
                  return 0;
               const auto& list = *list_ptr;
               for (size_t i = 0; i < list.size(); ++i) {
                  if (list[i] == v) {
                     lua_pushinteger(L, i + 1);
                     return 1;
                  }
               }
            }
            lua_pushnil(L);
            return 1;
         }
         
         template<typename Spec>
            requires (
               impl::concepts::is_fully_valid_spec<Spec>
            && impl::fields::get_collection_length::defaultable<Spec>
            && impl::concepts::values_are_subobjects<Spec>
            )
         int for_subobjects(lua_State* L) {
            auto& self     = Spec::pull_collection(L);
            auto  arg_type = lua_type(L, 2);
            
            if constexpr (impl::fields::pull_value::valid<Spec> || impl::fields::validate_value::valid<Spec>) {
               if (arg_type == LUA_TTABLE) {
                  cobb::lua::warning(L, "native_list<*>:index_of must be given a userdata to search for, not a table resembling the userdata");
                  lua_pushnil(L);
                  return 1;
               }
            }
            if (arg_type != LUA_TUSERDATA) {
               cobb::lua::warning(L, "userdata expected");
               lua_pushnil(L);
               return 1;
            }

            std::optional<size_t> arg_index_in_parent_coll = impl::get_item_wrapper_index(L, 2);
            if (!arg_index_in_parent_coll.has_value()) {
               lua_pushnil(L);
               return 1;
            }

            // HACK: Call the `get_item_by_index` function to access the item at the index that 
            //       our argument item claims to exist at. Then, compare the two items by userdata 
            //       pointer, since our wrapper system takes care to reuse those.
            lua_settop(L, 2);
            {
               lua_CFunction f;
               if constexpr (impl::fields::get_item_by_index::valid<Spec>) {
                  f = &Spec::get_item_by_index;
               } else {
                  static_assert(impl::fields::get_item_by_index::defaultable<Spec>);
                  f = &get_item_by_index<Spec>;
               }
               lua_pushcfunction(L, f);
            }
            lua_pushvalue(L, 1);
            lua_pushinteger(L, arg_index_in_parent_coll.value() + 1);
            lua_call(L, 1, 1);
            if (!lua_isuserdata(L, -1)) {
               lua_pushnil(L);
               return 1;
            }
            auto* argument_ud  = lua_touserdata(L, 2);
            auto* retrieved_ud = lua_touserdata(L, -1);
            if (argument_ud != retrieved_ud) {
               lua_pushnil(L);
               return 1;
            }
            lua_pushinteger(L, arg_index_in_parent_coll.value() + 1);
            return 1;
         }

         template<typename Spec>
         concept possible = requires {
            requires impl::concepts::is_fully_valid_spec<Spec>;
            requires impl::fields::get_collection_length::defaultable<Spec>;
            requires (impl::fields::pull_value::valid<Spec> || impl::concepts::values_are_subobjects<Spec>);
         };
      }
   }

   template<typename Spec>
      requires impl::index_of_value::possible<Spec>
   int index_of_value(lua_State* L) {
      if constexpr (impl::concepts::values_are_subobjects<Spec>) {
         return impl::index_of_value::for_subobjects<Spec>(L);
      } else {
         return impl::index_of_value::for_direct<Spec>(L);
      }
   }
}
