#pragma once
#include "../../../helpers/eight_cc.h"
#include <QTableView>
#include <QStandardItemModel>
#include "widget.h"

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc ui_table_view_cols = "TablCols"; // only used for the collection
   inline constexpr cobb::eight_cc ui_table_view_rows = "TablRows"; // only used for the collection
   inline constexpr cobb::eight_cc ui_table_view_span_cells = "TablSpan"; // only used for the row and col cell collections
}

namespace dovahscript::wrappers::ui {
   struct table_view : public widget {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.table_view";
      static constexpr const char*   class_name      = "table_view";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "table_view";
      using wrapped_type = QTableView;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L) noexcept;

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}