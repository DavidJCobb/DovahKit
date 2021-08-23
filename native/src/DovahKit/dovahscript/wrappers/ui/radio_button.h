#pragma once
#include <QRadioButton>
#include "widget.h"

namespace dovahscript::wrappers::ui {
   struct radio_button : public widget {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.radio_button";
      static constexpr const char*   class_name      = "radio_button";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "radio_button";
      using wrapped_type = QRadioButton;

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}