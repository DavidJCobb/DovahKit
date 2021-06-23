#pragma once
#include "../../../wrapper.h"
#include <QFont>

#include <type_traits>
#include "../../../../../helpers/function_traits.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc ui_font_data = "BaseFont"; // for widgets, canvas text data, etc.
   inline constexpr cobb::eight_cc ui_font_role = "FontRole"; // for model observers
}

namespace editor_script::impl::font_properties {
   struct handler {
      using get_function_t  = QVariant(*)(const QFont&);           // read  a value from a QFont
      using set_function_t  = void(*)(QFont&, const QVariant&);    // write a value into a QFont
      using push_function_t = int(*)(lua_State*, const QVariant&); // take a QFont value and push it to Lua. function should return the number of values pushed to the Lua stack
      using pull_function_t = QVariant(*)(lua_State*, int);        // take a Lua value and pull it to QFont. int argument is Lua stack pos; feel free to throw Lua errors

      const char*     name; // the field name we want to expose to Lua
      push_function_t push; // push a value into Lua
      pull_function_t pull; // pull a value from Lua
      get_function_t  get;  // write a value into a QFont
      set_function_t  set;  // read  a value from a QFont
      bool reset_if_nil = true;
      bool use_in_reset = true;
   };

   class handler_set : public std::vector<handler> {
      public:
         using std::vector<handler>::vector;

         inline const handler* lookup(const char* name) const noexcept {
            for (const auto& e : *this)
               if (cobb::strcmp(e.name, name) == 0)
                  return &e;
            return nullptr;
         }

         void extend_lua_class(lua_State* L, const char* class_metatable_key, int getter_list_stack_pos, int setter_list_stack_pos) const noexcept;

         QFont table_to_struct(lua_State* L, int table_pos) const noexcept;
   };
}

namespace editor_script::wrappers::ui {
   struct font : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.ui.font";
      static constexpr const char* class_name     = "font";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static const impl::font_properties::handler_set fph_handlers;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L) noexcept;

      using wrapped_type = QFont;

      // Given a Lua state and stack position, treat the value there as any supported way of specifying 
      // font properties (e.g. a Lua table; a userdata wrapping a different QFont; etc.), and return a 
      // QFont with those properties.
      static QFont pull(lua_State* L, int stack_pos);
   };
}