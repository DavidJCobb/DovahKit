#pragma once
#include "helpers/lua/error.h"
#include "helpers/lua/warning.h"
#include "../member_function_spec.h"

namespace dovahscript::api_helpers::native_lists::impl {
   template<typename Spec>
   void report_insertion_past_end(lua_State* L, const typename Spec::collection_wrapped_type& list, size_t insert_at, size_t subobject_index) {
      auto  size = list.size();
      if constexpr (Spec::allow_insertions_past_end) {
         if (insert_at > size) {
            cobb::lua::warning(L, "index %d is out of bounds; default elements will be created between the end of the list and the new element", subobject_index + 1);
         }
      } else {
         if (insert_at > size) {
            cobb::lua::error(L, "index %d is out of bounds; cannot insert past the end of the list", subobject_index + 1);
         }
      }
   }
}