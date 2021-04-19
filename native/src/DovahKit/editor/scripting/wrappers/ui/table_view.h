#pragma once
#include "../../wrapper.h"
#include "widget.h"
#include <QTableView>
#include <QStandardItemModel>

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc ui_table_view_cols = "TablCols"; // only used for the collection
   inline constexpr cobb::eight_cc ui_table_view_rows = "TablRows"; // only used for the collection
   inline constexpr cobb::eight_cc ui_table_view_span_cells = "TablSpan"; // only used for the row and col cell collections
}

namespace editor_script::wrappers::ui {
   struct table_view : public widget {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.table_view";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "table_view";
      using wrapped_type = QTableView;

      static constexpr const char* row_collection_key = "collection<dovah.classes.ui.table_view.rows>";
      static constexpr const char* col_collection_key = "collection<dovah.classes.ui.table_view.cols>";

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
   
   struct table_view_col : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.table_view_col";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "table_view_col";
      using wrapped_type = QStandardItem;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
   
   struct table_view_cell : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.table_view_cell";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "table_view_cell";
      using wrapped_type = QStandardItem;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}