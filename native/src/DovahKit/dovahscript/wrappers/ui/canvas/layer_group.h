#pragma once
#include "../../base.h"
#include "../../../wrapper.h"
#include "../../../../ui/generic/CanvasWidget.h"

namespace dovahscript::wrappers::ui {
   struct canvas_layer_group : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.canvas_layer_group";
      static constexpr const char*   class_name      = "canvas_layer_group";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = CanvasWidgetLayerGroup;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);
   };
}