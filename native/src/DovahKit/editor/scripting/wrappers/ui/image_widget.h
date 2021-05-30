#pragma once
#include "../../wrapper.h"
#include "widget.h"
#include "../../widgets/LuaManagedRasterWidget.h"

namespace editor_script::wrappers::ui {
   struct image_widget : public widget {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.image_widget";
      static constexpr const char* class_name     = "image_widget";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "image_widget";
      using wrapped_type = LuaManagedRasterWidget;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}