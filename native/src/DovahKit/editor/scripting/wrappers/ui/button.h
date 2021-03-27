#pragma once
#include "../../wrapper.h"
#include "widget.h"
#include <QPushButton>

namespace editor_script::wrappers::ui {
   struct button : public widget {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.ui.button";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr char* global_name = "button";
      using wrapped_type = QPushButton;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}