#pragma once
#include "../../../wrapper.h"
#include <QTableView>
#include <QStandardItemModel>
#include "../table_view.h"

namespace editor_script::wrappers::ui {
   struct table_view_row : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.table_view_row";
      static constexpr const char* class_name     = "table_view_row";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "table_view_row";
      using wrapped_type = QStandardItem;

      static constexpr const char* cell_collection_key = "collection<dovah.classes.ui.table_view_row.cells>";

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}