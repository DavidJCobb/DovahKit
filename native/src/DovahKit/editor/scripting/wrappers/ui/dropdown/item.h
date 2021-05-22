#pragma once
#include "../../../wrapper.h"
#include <QComboBox>
#include <QStandardItemModel>

#include "../helpers/model_observer_data.h"

namespace editor_script::wrappers::ui {
   struct dropdown_item : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.dropdown_item";
      static constexpr const char* class_name     = "dropdown_item";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static const moph::handler_set moph_handlers;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L) noexcept;

      static constexpr const char* global_name = "dropdown_item";
      using wrapped_type = QStandardItem;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}