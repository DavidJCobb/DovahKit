#pragma once
#include "lua.h"
#include "./impl/concepts/is_fully_valid_spec.h"
#include "./impl/concepts/storage_is_bifurcated.h"
#include "./impl/fields/get_collection_length.h"

namespace dovahscript::api_helpers::native_lists {
   template<typename Spec>
      requires (impl::concepts::is_fully_valid_spec<Spec> && impl::fields::get_collection_length::defaultable<Spec>)
   int get_collection_length(lua_State* L) {
      auto& self = Spec::pull_collection(L);
      if constexpr (impl::concepts::storage_is_bifurcated<Spec>) {
         const auto pair = Spec::unwrap_collection(self);
         size_t size = 0;
         if (pair.first)
            size = pair.first->size();
         if (pair.second)
            size += pair.second->size();
         lua_pushinteger(L, size);
         return 1;
      } else {
         const auto* list_ptr = Spec::unwrap_collection(self);
         if (!list_ptr)
            return 0;
         lua_pushinteger(L, list_ptr->size());
         return 1;
      }
   }
}
