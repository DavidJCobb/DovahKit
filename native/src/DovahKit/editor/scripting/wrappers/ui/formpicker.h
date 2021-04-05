#pragma once
#include "../../wrapper.h"
#include "widget.h"
#include "../../../ui/generic/FormPicker.h"

namespace editor_script::wrappers::ui {
   struct formpicker : public widget {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.ui.formpicker";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr char* global_name = "formpicker";
      using wrapped_type = FormPicker;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}