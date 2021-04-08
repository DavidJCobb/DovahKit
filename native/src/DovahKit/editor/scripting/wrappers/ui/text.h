#pragma once
#include "../../wrapper.h"
#include "widget.h"
#include <QLabel>

namespace editor_script::wrappers::ui {
   struct text : public widget {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.text";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "text";
      using wrapped_type = QLabel;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}