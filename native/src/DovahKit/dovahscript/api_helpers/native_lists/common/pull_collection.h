#pragma once
#include <string_view>
#include "helpers/lua/error.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/wrapper.h"

namespace dovahscript::api_helpers::native_lists::common {
   template<const std::string_view& CollectionMetatableKey>
   wrapper& pull_collection(lua_State* L) {
      auto* self = (wrapper*)classes::cast_to_class(L, 1, CollectionMetatableKey.data());
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", CollectionMetatableKey.data());
      }
      return *self;
   }
}