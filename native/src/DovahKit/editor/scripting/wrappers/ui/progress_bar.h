#pragma once
#include "../../wrapper.h"
#include "widget.h"
#include <QProgressBar>

namespace editor_script::wrappers::ui {
   struct progress_bar : public widget {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.ui.progress_bar";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr char* global_name = "progress_bar";
      using wrapped_type = QProgressBar;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}