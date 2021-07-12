#pragma once
#include "../landscape.h"

namespace editor_script::wrappers {
   //
   // landscape.quads.top_left.default_texture
   // landscape.quads.top_left.layers[1]
   //
   struct landscape_quad : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.landscape_quad";
      static constexpr const char* class_name     = "landscape_quad";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;
   };
}