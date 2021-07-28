#pragma once
#include "../base.h"
#include <QWidget>

namespace dovahscript::wrappers::ui {
   struct widget : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.widget";
      static constexpr const char*   class_name      = "widget";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "widget";
      using wrapped_type = QWidget;

      static void setup(lua_State*); // the "ui" table should be at the top of the stack
   };
}