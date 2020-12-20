#pragma once
#include "../../../Lua/lua.hpp"

namespace editor_script::classes {
   class _base {
      public:

         //
         // TODO: The VM should retain a vector of userdata, and it should only be 
         // possible to instantiate these things while at the same time adding them 
         // to that vector.
         //
   };

   template<typename T> T* class_from_stack(lua_State* L, int pos) {
      return (T*)editor_script::cast_to_class(L, pos, T::metatable_key);
   }
}