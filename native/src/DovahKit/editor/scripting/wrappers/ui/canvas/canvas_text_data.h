#pragma once
#include "../../../wrapper.h"
#include "../../../widgets/objects/CanvasWidgetLayerDataLuaText.h"

namespace editor_script::wrappers::ui {
   struct canvas_text_data : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.canvas_text_data";
      static constexpr const char* class_name     = "canvas_text_data";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "canvas_text_data";
      using wrapped_type = CanvasWidgetLayerDataLuaText;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}