#pragma once
#include "../../../wrapper.h"
#include <QTableView>
#include <QStandardItemModel>
#include "../table_view.h"

namespace editor_script::wrappers::ui {
   struct table_view_cell : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.table_view_cell";
      static constexpr const char* class_name     = "table_view_cell";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L) noexcept;

      static constexpr const char* global_name = "table_view_cell";
      using wrapped_type = QStandardItem;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}