#pragma once
#include "widgets/DKProgressBar.h"
#include "./widget.h"

namespace dovahscript::wrappers::ui {
   struct progress_bar : public widget {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.progress_bar";
      static constexpr const char*   class_name      = "progress_bar";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "progress_bar";
      using wrapped_type = DKProgressBar;

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}