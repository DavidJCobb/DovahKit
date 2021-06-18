#pragma once
#include "../../../wrapper.h"
#include <QFont>

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc ui_font_data = "BaseFont"; // for widgets, canvas text data, etc.
   inline constexpr cobb::eight_cc ui_font_role = "FontRole"; // for model observers
}

namespace editor_script::wrappers::ui {
   struct font : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.font";
      static constexpr const char* class_name     = "font";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_type = QFont;

      // Given a Lua state and stack position, treat the value there as any supported way of specifying 
      // font properties (e.g. a Lua table; a userdata wrapping a different QFont; etc.), and return a 
      // QFont with those properties.
      static QFont pull(lua_State* L, int stack_pos);
   };
}