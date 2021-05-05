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
      extern int fail_to_push(lua_State* L, const QVariant&);
      extern QVariant fail_to_pull(lua_State* L, int);
   }

   struct model_observer_property_handler {
      using push_function_t = int(*)(lua_State*, const QVariant&); // return number of values pushed to the Lua stack
      using pull_function_t = QVariant(*)(lua_State*, int); // int argument is Lua stack pos

      const char*      name; // the field name we want to expose to Lua
      Qt::ItemDataRole role;
      push_function_t  push; // push a value into Lua
      pull_function_t  pull; // pull a value from Lua

      inline constexpr bool is_valid() const noexcept { return this->name && (int)this->role >= 0; }
   };
   extern constexpr model_observer_property_handler invalid_handler = { "", (Qt::ItemDataRole)-1, util::fail_to_push, util::fail_to_pull };

   class handler_set : public std::vector<model_observer_property_handler> {
      public:
         using std::vector<model_observer_property_handler>::vector;

         const model_observer_property_handler* lookup(const char* name) const noexcept {
            for (const auto& e : *this)
               if (cobb::strcmp(e.name, name) == 0)
                  return &e;
            return nullptr;
         }
   };

   extern QMap<Qt::ItemDataRole, QVariant> extract_role_dataset_from_table(lua_State* L, int table_pos, const model_observer_property_handler* const list, int size);
   template<int I> extern QMap<Qt::ItemDataRole, QVariant> extract_role_dataset_from_table(lua_State* L, int table_pos, const std::array<model_observer_property_handler, I>& list) {
      extract_role_dataset_from_table(L, table_pos, list.data(), list.size());
   }

   template<typename mt> extern int getter(lua_State* L) {
      // Upvalue 1: light userdata: the handler set instance
      // Upvalue 2: light userdata: the handler name
      auto& self = get_wrapper_for_thiscall<mt>(L);
      if (!self.model_observer)
         return 0;
      assert(lua_islightuserdata(L, lua_upvalueindex(1)));
      assert(lua_isstring(L, lua_upvalueindex(2)));
      auto* hset = (handler_set*) lua_touserdata(L, lua_upvalueindex(1));
      auto* name = lua_tostring(L, lua_upvalueindex(2));
      assert(hset && name && name[0]);
      auto* moph = hset->lookup(name);
      if (!moph) {
         return luaL_error(L, "property `%1` is not available here", name);
      }
      QVariant result = helpers::get_model_items_data(self.model_observer, moph->role);
      return moph->push(L, result);
   }
   template<typename mt> extern int setter(lua_State* L) {
      // Upvalue 1: light userdata: the handler set instance
      // Upvalue 2: light userdata: the handler name
      // Upvalue 3: boolean:        clear if invalid
      auto& self = get_wrapper_for_thiscall<mt>(L);
      if (!self.model_observer)
         return 0;
      assert(lua_islightuserdata(L, lua_upvalueindex(1)));
      assert(lua_isstring(L, lua_upvalueindex(2)));
      auto* hset = (handler_set*) lua_touserdata(L, lua_upvalueindex(1));
      auto* name = lua_tostring(L, lua_upvalueindex(2));
      assert(hset && name && name[0]);
      auto* moph = hset->lookup(name);
      if (!moph) {
         return luaL_error(L, "property `%1` is not available here", name);
      }
      QVariant value = moph->pull(L, 2);
      if (!value.isValid()) {
         if (!lua_toboolean(L, lua_upvalueindex(3))) {
            return luaL_error(L, "the value is invalid"); // TODO: can we report specific errors?
         }
      }
      helpers::set_model_items_data(self.model_observer, moph->role, value);
      return 0;
   }

   template<typename mt> int push_getter(lua_State* L, const handler_set& list, const char* name) {
      assert(list.lookup(name));
      lua_pushlightuserdata(L, (void*)&list);
      lua_pushstring(L, name);
      lua_pushcclosure(L, &getter<mt>, 2);
      return 1;
   }
   template<typename mt> int push_setter(lua_State* L, const handler_set& list, const char* name, bool clear_if_invalid = false) {
      assert(list.lookup(name));
      lua_pushlightuserdata(L, (void*)&list);
      lua_pushstring(L, name);
      lua_pushboolean(L, clear_if_invalid);
      lua_pushcclosure(L, &setter<mt>, 3);
      return 1;
   }

   extern int push_alignment(lua_State*, const QVariant&);
   extern QVariant pull_alignment(lua_State*, int stack_pos);

   extern int push_color(lua_State*, const QVariant&);
   extern QVariant pull_color(lua_State*, int stack_pos);

   extern int push_string(lua_State*, const QVariant&);
   extern QVariant pull_string(lua_State*, int stack_pos);
}