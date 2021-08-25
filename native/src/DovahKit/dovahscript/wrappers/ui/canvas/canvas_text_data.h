#pragma once
#include "../../base.h"
#include "../../../wrapper.h"
#include "../../../qt/DovahscriptCanvasWidgetLayerDataText.h"

namespace dovahscript::wrappers::ui {
   struct canvas_text_data : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.canvas_text_data";
      static constexpr const char*   class_name      = "canvas_text_data";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "canvas_text_data";
      using wrapped_type = DovahscriptCanvasWidgetLayerDataText;

      static void import_singleton(lua_State*); // the "ui" table should be at the top of the stack
   };
}