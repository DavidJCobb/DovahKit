#pragma once
#include "../../wrapper.h"
#include <QButtonGroup>

namespace editor_script::wrappers::ui {
   struct radio_group : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.radio_group";
      static constexpr const char* class_name     = "radio_group";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "radio_group";
      using wrapped_type = QButtonGroup;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}
