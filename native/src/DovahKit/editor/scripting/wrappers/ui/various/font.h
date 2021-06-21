#pragma once
#include "../../../wrapper.h"
#include <QFont>

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc ui_font_data = "BaseFont"; // for widgets, canvas text data, etc.
   inline constexpr cobb::eight_cc ui_font_role = "FontRole"; // for model observers
}

namespace editor_script::wrappers::ui {
   namespace impl {
      struct font_property_handler {
         using push_function_t  = int(*)(lua_State*, const QFont&); // return number of values pushed to the Lua stack
         using pull_function_t  = QVariant(*)(lua_State*, int);     // int argument is Lua stack pos. feel free to throw Lua errors
         using apply_function_t = QFont(*)(QFont, const QVariant&); // apply a pulled property to a QFont

         const char*      name;  // the field name we want to expose to Lua
         push_function_t  push;  // push a value into Lua
         pull_function_t  pull;  // pull a value from Lua
         apply_function_t apply;
      };
      class font_handler_set : public std::vector<font_property_handler> {
         public:
            using role_map_t = QMap<Qt::ItemDataRole, QVariant>;
         public:
            using std::vector<font_property_handler>::vector;

            const font_property_handler* lookup(const char* name) const noexcept;
            void extend(lua_State* L, const char* class_metatable_key, int getter_list_stack_pos, int setter_list_stack_pos) const noexcept;

            QFont extract(lua_State* L, int table_pos) const noexcept;
      };
   }

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