#pragma once
#include "form.h"

#include "../../../dovah/forms/Landscape.h"

namespace editor_script::wrappers {
   struct landscape : public form {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.landscape";
      static constexpr const char* class_name     = "landscape";
      static std::initializer_list<luaL_Reg> metatable_methods;
      static std::initializer_list<luaL_Reg> metatable_getters;
      static std::initializer_list<luaL_Reg> metatable_setters;
   };
}