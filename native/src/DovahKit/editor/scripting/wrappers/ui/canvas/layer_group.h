#pragma once
#include "../../../wrapper.h"
#include "../../../../../ui/generic/CanvasWidget.h"
#include "../canvas.h"

namespace editor_script::wrappers::ui {
   struct canvas_layer_group : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.canvas_layer_group";
      static constexpr const char* class_name     = "canvas_layer_group";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_type = CanvasWidgetLayerGroup;

      static constexpr const char* layer_collection_key = "collection<dovah.classes.ui.canvas_layer_group.layers>";

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}