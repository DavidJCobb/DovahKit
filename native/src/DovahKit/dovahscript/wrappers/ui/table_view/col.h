#pragma once
#include "../../base.h"
#include "../../../wrapper.h"

namespace dovahscript::wrappers::ui {
   struct table_view_col : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.ui.table_view_col";
      static constexpr const char*   class_name      = "table_view_col";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "table_view_col";

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L) noexcept;

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}