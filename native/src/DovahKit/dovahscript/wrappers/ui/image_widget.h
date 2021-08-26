#pragma once
#include "../../qt/DovahscriptImageWidget.h"
#include "widget.h"

namespace dovahscript::wrappers::ui {
   struct image_widget : public widget {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.image_widget";
      static constexpr const char*   class_name      = "image_widget";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "image_widget";
      using wrapped_type = DovahscriptImageWidget;

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}