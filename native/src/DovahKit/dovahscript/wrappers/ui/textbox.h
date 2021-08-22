#pragma once
#include "widget.h"
#include <QLineEdit>

namespace dovahscript::wrappers::ui {
   //
   // Current plan is to use the term "textbox" for a one-line textbox, and 
   // "textarea" for a multi-line textbox.
   //
   struct textbox : public widget {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.textbox";
      static constexpr const char*   class_name      = "textbox";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "textbox";
      using wrapped_type = QLineEdit;

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}