#pragma once
#include "../../wrapper.h"
#include <QDialog>

namespace editor_script::wrappers::ui {
   struct window : public wrapper_metatable {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.ui.window";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;
      using wrapped_type = QDialog;

      inline static bool check_arg_type(lua_State* L, int i) {
         return editor_script::check_for_class(L, i, metatable_key);
      }
      inline static void require_arg_type(lua_State* L, int i) {
         if (!editor_script::check_for_class(L, i, metatable_key))
            luaL_error(L, "bad argument #%i (expected %s)", i, metatable_key);
      }
      inline static void require_self_type(lua_State* L) {
         if (!editor_script::check_for_class(L, 1, metatable_key))
            luaL_error(L, "function called with bad self (expected %s)", metatable_key);
      }

      static void setup(lua_State*);
   };
}