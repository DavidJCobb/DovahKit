#pragma once
#include "../../wrapper.h"

namespace editor_script::wrappers {
   struct papyrus_root : public wrapper_metatable {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.papyrus_root";
      static luaL_Reg metatable_methods[];
      static luaL_Reg metatable_getters[];
      static luaL_Reg metatable_setters[];
   };
   struct papyrus_script : public wrapper_metatable {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.papyrus_script";
      static luaL_Reg metatable_methods[];
      static luaL_Reg metatable_getters[];
      static luaL_Reg metatable_setters[];
   };
}