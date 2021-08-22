#pragma once
#include "../../qt/DovahscriptLineWidget.h"
#include "widget.h"

namespace dovahscript::wrappers::ui {
   struct line : public widget {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.line";
      static constexpr const char*   class_name      = "line";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "line";
      using wrapped_type = DovahscriptLineWidget;

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}