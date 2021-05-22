#pragma once
#include "../../wrapper.h"
#include "widget.h"
#include <QComboBox>
#include <QStandardItemModel>

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc ui_dropdown_items = "DpdnItem"; // only used for the collection
}

namespace editor_script::wrappers::ui {
   struct dropdown : public widget {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.dropdown";
      static constexpr const char* class_name     = "dropdown";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr const char* global_name = "dropdown";
      using wrapped_type = QComboBox;

      static constexpr const char* item_collection_key = "collection<dovah.classes.ui.dropdown.items>";

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}