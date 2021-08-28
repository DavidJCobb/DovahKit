#include "collection_entries.h"
#include "../../../../helpers/lua/error.h"
#include "../../../../helpers/lua/warning.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../push_native_object.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/FormList.h"
#include "../formlist.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.formlist.entries>";
}

namespace {
   using namespace dovahscript;

   using wrapped_type = dovah::loaded_forms::FormList;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      }
      return *self;
   }
   
   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      lua_pushinteger(L, form->contents.size());
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      auto  i    = lua_tointeger(L, 2);
      auto& list = form->contents;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      return push_native_object(list[i]);
   }
   int member_function_insert(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      //
      auto& self  = get_collection_wrapper(L);
      auto* form  = self.get_loaded_form_data<wrapped_type>();
      //
      int  pos_value = 2;
      bool has_index = false;
      //
      if (lua_gettop(L) >= 3) {
         has_index = true;
         pos_value = 3;
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "provided index is not an integer");
      }
      dovah::form_stub* target = nullptr;
      if (!lua_isnoneornil(L, pos_value)) {
         auto* w = wrapper_from_stack<wrappers::form>(L, pos_value);
         if (!w)
            cobb::lua::error(L, "you can only insert forms or nil into a formlist");
         target = w->stub;
      }
      //
      if (!form)
         return 0;
      auto& list = form->contents;
      auto  size = list.size();
      int   i    = size + 1;
      if (has_index) {
         i = lua_tointeger(L, 2);
         if (i < 1)
            cobb::lua::error(L, "indices below 1, such as %d, are not allowed", i);
         --i;
      }
      if (i >= size) {
         if (i > size) {
            cobb::lua::warning(L, "index %s is out of bounds; nil elements will be created between the end of the list and the new element", lua_tolstring(L, 2, nullptr));
         }
         list.resize(i + 1);
      } else {
         list.emplace(list.begin() + i);
      }
      self.before_edit();
      list[i].set(*form, target);
      self.after_edit();
      return 0;
   }
   int member_function_remove(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      //
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected an integer index");
      int isnum;
      int i = lua_tointegerx(L, 2, &isnum);
      luaL_argcheck(L, isnum, 2, "expected an integer index");
      if (!form)
         return 0;
      auto& list = form->contents;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      self.before_edit();
      list[i].set(*form, nullptr);
      list.erase(list.begin() + i);
      self.after_edit();
      return 0;
   }
   int set_item(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      //
      constexpr auto index_self  = 1;
      constexpr auto index_key   = 2;
      constexpr auto index_value = 3;
      //
      dovah::form_stub* target = nullptr;
      if (!lua_isnoneornil(L, index_value)) {
         auto* w = wrapper_from_stack<wrappers::form>(L, index_value);
         if (!w)
            cobb::lua::error(L, "you can only overwrite formlist entries with forms or nil");
         target = w->stub;
      }
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      int v = 0;
      int i = lua_tointegerx(L, index_key, &v);
      if (!v)
         cobb::lua::error(L, "indices in a formlist's entry list must be integers");
      if (i < 1)
         cobb::lua::error(L, "indices below 1, such as %d, are not allowed", i);
      --i;
      //
      auto& list = form->contents;
      auto  size = list.size();
      if (i >= size) {
         if (i > size) {
            cobb::lua::warning(L, "index %s is out of bounds; nil elements will be created between the end of the list and the new element", lua_tolstring(L, index_key, nullptr));
         }
         list.resize(i + 1);
      }
      self.before_edit();
      list[i].set(*form, target);
      self.after_edit();
      return 0;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params quest_alias_by_id_set = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
      .member_function_insert = &member_function_insert,
      .member_function_remove = &member_function_remove,
      .set_item               = &set_item,
   };
}