#pragma once
#include "../texture_set.h"

namespace editor_script::wrappers {
   struct texture_set_path_list : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.texture_set_path_list";
      static constexpr const char* class_name     = "texture_set_path_list";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;
   };
}