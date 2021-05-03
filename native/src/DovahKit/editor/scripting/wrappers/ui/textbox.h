#pragma once
#include "../../wrapper.h"
#include "widget.h"
#include <QLineEdit>

namespace editor_script::wrappers::ui {
   //
   // Current plan is to use the term "textbox" for a one-line textbox, and 
   // "textarea" for a multi-line textbox.
   //
   struct textbox : public widget {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.textbox";
      static constexpr const char* class_name     = "textbox";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "textbox";
      using wrapped_type = QLineEdit;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}