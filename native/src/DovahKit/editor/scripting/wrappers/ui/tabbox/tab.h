#pragma once
#include "../../../wrapper.h"
#include <QTableView>
#include <QStandardItemModel>
#include "../table_view.h"

namespace editor_script::wrappers::ui {
   struct tabbox_tab : public widget {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.tabbox_tab";
      static constexpr const char* class_name     = "tabbox_tab";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "tabbox_tab";
      using wrapped_type = QWidget;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}