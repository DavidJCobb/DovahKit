#pragma once
#include <functional>
#include <QVariant>
#include "../../lua.h"
#include "../../helpers/strings.h"

namespace dovahscript {
   class wrapper;
}

namespace dovahscript::api_helpers::moph {
   namespace util {
      extern int getter(lua_State* L);
      extern int setter(lua_State* L);
   }

   struct model_observer_property_handler {
      using push_function_t      = int(*)(lua_State*, const QVariant& value, const wrapper& observer); // Function to push a value into Lua. Return the number of values pushed to the Lua stack.
      using pull_function_t      = QVariant(*)(lua_State*, int stack_pos);                             // Function to pull a value from Lua. Int argument is Lua stack pos. Feel free to throw Lua errors.
      using transform_function_t = QVariant(*)(const QVariant& prior, const QVariant& changes);

      const char*          name; // the field name we want to expose to Lua
      Qt::ItemDataRole     role;
      push_function_t      push; // push a value into Lua
      pull_function_t      pull; // pull a value from Lua
      transform_function_t transform = nullptr; // optional function for if we want to modify an existing value rather than overwrite it
      //
      bool clear_if_invalid = false;

      inline constexpr bool is_valid() const noexcept { return this->name && (int)this->role >= 0; }

      static constexpr const transform_function_t default_transform = nullptr;
   };

   class handler_set : public std::vector<model_observer_property_handler> {
      public:
         using role_map_t = QMap<Qt::ItemDataRole, QVariant>;
      public:
         using std::vector<model_observer_property_handler>::vector;

         const model_observer_property_handler* lookup(const char* name) const noexcept {
            for (const auto& e : *this)
               if (cobb::strcmp(e.name, name) == 0)
                  return &e;
            return nullptr;
         }

         void extend(lua_State* L, const char* class_metatable_key, int getter_list_stack_pos, int setter_list_stack_pos) const noexcept;

         role_map_t extract(lua_State* L, int table_pos) const noexcept;
   };

   extern int push_alignment(lua_State*, const QVariant&, const wrapper& observer);
   extern QVariant pull_alignment(lua_State*, int stack_pos);
   extern QVariant transform_alignment(const QVariant& prior, const QVariant& changes);

   extern int push_color(lua_State*, const QVariant&, const wrapper& observer);
   extern QVariant pull_color(lua_State*, int stack_pos);

   // Reads return either nil or a wrappers::ui::font wrapper. Writes accept nil, a table, or a wrapper.
   extern int push_font(lua_State*, const QVariant&, const wrapper& observer);
   extern QVariant pull_font(lua_State*, int stack_pos);

   // Icons can be QColors or rasters, currently.
   extern int push_icon(lua_State*, const QVariant&, const wrapper& observer);
   extern QVariant pull_icon(lua_State*, int stack_pos);

   extern int push_string(lua_State*, const QVariant&, const wrapper& observer);
   extern QVariant pull_string(lua_State*, int stack_pos);
}