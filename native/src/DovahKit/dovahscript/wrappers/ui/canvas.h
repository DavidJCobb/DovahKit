#pragma once
#include "../../../ui/generic/CanvasWidget.h"
#include "../../../helpers/eight_cc.h"
#include "widget.h"

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc ui_canvas_layers = "CanvLayr"; // only used for the collection
}

namespace dovahscript::wrappers::ui {
   struct canvas : public widget {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.canvas";
      static constexpr const char*   class_name      = "canvas";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "canvas";
      using wrapped_type = CanvasWidget;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);

      static void import_singleton(lua_State*); // the "ui" table should be at the top of the stack
   };
}