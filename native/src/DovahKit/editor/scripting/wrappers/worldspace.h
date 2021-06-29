#pragma once
#include "form.h"

namespace editor_script::wrappers {
   struct worldspace : public form {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.worldspace";
      static constexpr const char* class_name     = "worldspace";
      static std::initializer_list<luaL_Reg> metatable_methods;
      static std::initializer_list<luaL_Reg> metatable_getters;
      static std::initializer_list<luaL_Reg> metatable_setters;
   };
}