#pragma once
#include "../../wrapper.h"
#include "widget.h"
#include <QTabWidget>

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc ui_tabbox_tabs = "TabBoxTb";
}

namespace editor_script::wrappers::ui {
   struct tabbox : public widget {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.tabbox";
      static constexpr const char* class_name     = "tabbox";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "tabbox";
      using wrapped_type = QTabWidget;

      static constexpr const char* tab_collection_key = "collection<dovah.classes.ui.tabbox.tabs>";

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}