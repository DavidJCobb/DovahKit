#pragma once
#include "../../wrapper.h"

namespace editor_script::wrappers::resource {
   struct raster : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.resource.raster";
      static constexpr const char* class_name     = "raster";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "raster";

      static void setup(lua_State*);

      static int wrap_and_push(lua_State*, LuaManagedResource&);
   };
}