#pragma once
#include <functional>
#include "../../../ui/util/lua_item_model.h"
#include "../../../../../lua.h"
#include "../../../../../helpers/strings.h"

namespace editor_script::helpers {
   extern [[nodiscard]] QVariant get_model_items_data(ObservableStandardItemModelObserver* observer, int role);
   extern void set_model_items_data(ObservableStandardItemModelObserver* observer, int role, QVariant data);

   // Row/column numbers of -2 indicate "remove the whole span." Both == -2 indicate "clear the whole model."
   extern void remove_items_from_model(QWidget* widget, int row, int col, QModelIndex parent = QModelIndex());
}

namespace editor_script::moph {
   namespace util {
      extern int getter(lua_State* L);
      extern int setter(lua_State* L);
   }

   struct model_observer_property_handler {
      using push_function_t      = int(*)(lua_State*, const QVariant&); // return number of values pushed to the Lua stack
      using pull_function_t      = QVariant(*)(lua_State*, int); // int argument is Lua stack pos. feel free to throw Lua errors
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

   extern int push_alignment(lua_State*, const QVariant&);
   extern QVariant pull_alignment(lua_State*, int stack_pos);
   extern QVariant transform_alignment(const QVariant& prior, const QVariant& changes);

   extern int push_color(lua_State*, const QVariant&);
   extern QVariant pull_color(lua_State*, int stack_pos);

   // Icons can be QColors or rasters, currently.
   extern int push_icon(lua_State*, const QVariant&);
   extern QVariant pull_icon(lua_State*, int stack_pos);

   extern int push_string(lua_State*, const QVariant&);
   extern QVariant pull_string(lua_State*, int stack_pos);
}