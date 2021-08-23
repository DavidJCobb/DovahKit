#pragma once
#include <QButtonGroup>
#include "../base.h"

namespace dovahscript::wrappers::ui {
   struct radio_group : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.radio_group";
      static constexpr const char*   class_name      = "radio_group";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "radio_group";
      using wrapped_type = QButtonGroup;

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}
