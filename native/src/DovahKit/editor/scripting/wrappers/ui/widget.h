#pragma once
#include "../../wrapper.h"
#include <QWidget>

namespace editor_script::wrappers::ui {
   struct widget : public wrapper_metatable {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.ui.widget";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr char* global_name = "widget";
      using wrapped_type = QWidget;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}