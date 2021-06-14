#pragma once
#include "../../../wrapper.h"
#include "../../../../../ui/generic/CanvasWidget.h"

namespace editor_script::wrappers::ui {
   struct canvas_layer : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.canvas_layer";
      static constexpr const char* class_name     = "canvas_layer";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_type = CanvasWidgetLayer;
   };
}