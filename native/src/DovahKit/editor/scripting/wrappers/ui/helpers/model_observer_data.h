#pragma once
#include <functional>
#include "../../../ui/util/lua_item_model.h"
#include "../../../../../lua.h"
#include "../../../../../helpers/strings.h"

namespace editor_script::helpers {
   extern [[nodiscard]] QVariant get_model_items_data(ObservableStandardItemModelObserver* observer, int role);
   extern void set_model_items_data(ObservableStandardItemModelObserver* observer, int role, QVariant data);
}

namespace editor_script::moph {
   namespace util {
      extern int getter(lua_State* L);
      extern int setter(lua_State* L);
   }

   struct model_observer_property_handler {
      using push_function_t = int(*)(lua_State*, const QVariant&); // return number of values pushed to the Lua stack
      using pull_function_t = QVariant(*)(lua_State*, int); // int argument is Lua stack pos

      const char*      name; // the field name we want to expose to Lua
      Qt::ItemDataRole role;
      push_function_t  push; // push a value into Lua
      pull_function_t  pull; // pull a value from Lua
      //
      bool clear_if_invalid = false;

      inline constexpr bool is_valid() const noexcept { return this->name && (int)this->role >= 0; }
   };

   class handler_set : public std::vector<model_observer_property_handler> {
      public:
         using std::vector<model_observer_property_handler>::vector;

         const model_observer_property_handler* lookup(const char* name) const noexcept {
            for (const auto& e : *this)
               if (cobb::strcmp(e.name, name) == 0)
                  return &e;
            return nullptr;
         }

         void extend(lua_State* L, const char* class_metatable_key, int getter_list_stack_pos, int setter_list_stack_pos) const noexcept;

         QMap<Qt::ItemDataRole, QVariant> extract(lua_State* L, int table_pos) const noexcept;
   };

   extern int push_alignment(lua_State*, const QVariant&);
   extern QVariant pull_alignment(lua_State*, int stack_pos);

   extern int push_color(lua_State*, const QVariant&);
   extern QVariant pull_color(lua_State*, int stack_pos);

   extern int push_string(lua_State*, const QVariant&);
   extern QVariant pull_string(lua_State*, int stack_pos);
}