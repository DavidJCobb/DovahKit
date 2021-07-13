#pragma once
#include "form.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc texture_set_paths = "TxStPath";
}

namespace editor_script::wrappers {
   struct texture_set : public form {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.texture_set";
      static constexpr const char* class_name     = "texture_set";
      static std::initializer_list<luaL_Reg> metatable_methods;
      static std::initializer_list<luaL_Reg> metatable_getters;
      static std::initializer_list<luaL_Reg> metatable_setters;
   };
}