#pragma once
#include "widgets/DKColorPickerButton.h"
#include "widget.h"

namespace dovahscript::wrappers::ui {
   struct color_button : public widget {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.color_button";
      static constexpr const char*   class_name      = "color_button";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "color_button";
      using wrapped_type = DKColorPickerButton;

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}