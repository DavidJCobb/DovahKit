#pragma once
#include <QTabWidget>
#include "../../../helpers/eight_cc.h"
#include "widget.h"

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc ui_tabbox_tabs = "TabBoxTb";
}

namespace dovahscript::wrappers::ui {
   struct tabbox : public widget {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.tabbox";
      static constexpr const char*   class_name      = "tabbox";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "tabbox";
      using wrapped_type = QTabWidget;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}