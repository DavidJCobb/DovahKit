#include "collection_array_values.h"
#include "../../../../../helpers/lua/error.h"
#include "../../../../../helpers/lua/warning.h"
#include "../../../../core/subsystems/permissions.h"
#include "../../../../core/subsystems/userdata.h"
#include "../../../../core/classes.h"
#include "../../../../push_native_object.h"
#include "../../../../wrapper.h"

#include "../../../../../dovah/forms/Quest.h"
#include "../root.h"
#include "../script.h"
#include "../property.h"
#include "../../quest/alias.h"
#include "../../../../api_helpers/papyrus_property_values.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.papyrus_property[n]>";
}

namespace {
   using namespace dovahscript;

   using root_wrapper_t   = dovahscript::wrappers::papyrus_root;
   using script_wrapper_t = dovahscript::wrappers::papyrus_script;
   using prop_wrapper_t   = dovahscript::wrappers::papyrus_property;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      }
      return *self;
   }

   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* prop = prop_wrapper_t::unwrap(self, false);
      if (!prop) {
         lua_pushinteger(L, 0);
         return 1;
      }
      using pt = dovah::loaded_forms::components::papyrus::property_type;
      switch (prop->type) {
         case pt::boolean:
         case pt::float32:
         case pt::string:
         case pt::integer:
         case pt::object:
            lua_pushinteger(L, 0);
            return 1;
         case pt::array_of_boolean:
         case pt::array_of_float32:
         case pt::array_of_string:
         case pt::array_of_integer:
         case pt::array_of_object:
            lua_pushinteger(L, prop->values.size());
            return 1;
      }
      lua_pushinteger(L, 0);
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* prop = prop_wrapper_t::unwrap(self, false);
      if (!prop)
         return 0;
      auto  i    = lua_tointeger(L, 2);
      auto& list = prop->values;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      //
      using pt = dovah::loaded_forms::components::papyrus::property_type;
      switch (prop->type) {
         case pt::array_of_boolean:
            lua_pushboolean(L, prop->values[i].boolean);
            return 1;
         case pt::array_of_float32:
            lua_pushnumber(L, prop->values[i].float32);
            return 1;
         case pt::array_of_string:
            lua_pushstring(L, prop->values[i].string.c_str());
            return 1;
         case pt::array_of_integer:
            lua_pushinteger(L, prop->values[i].integer);
            return 1;
         case pt::array_of_object:
            {
               auto& value = prop->values[i].object;
               auto* quest = value.form.get_form_stub();
               //
               if (quest) {
                  if (value.aliasID != (decltype(value.aliasID))dovah::loaded_forms::Alias::none_id) {
                     return wrappers::quest_alias::wrap(L, quest, value.aliasID);
                  } else {
                     return push_native_object(quest);
                  }
               }
               return 0;
            }
      }
      return 0;
   }
   int member_function_insert(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      //
      auto& self = get_collection_wrapper(L);
      auto* prop = prop_wrapper_t::unwrap(self, false);
      //
      int  pos_value = 2;
      bool has_index = false;
      //
      if (lua_gettop(L) >= 3) {
         has_index = true;
         pos_value = 3;
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "provided index is not an integer");
      }
      if (!dovahscript::api_helpers::papyrus::property_scalar_value_typecheck(L, pos_value, prop->type))
         cobb::lua::error(L, "desired value is of the wrong type for this property");
      //
      if (!prop)
         return 0;
      auto& list = prop->values;
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
      dovahscript::api_helpers::papyrus::set_property_value(L, pos_value, self, *prop, i);
      self.after_edit();
      return 0;
   }
   int member_function_remove(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      //
      auto& self = get_collection_wrapper(L);
      auto* prop = prop_wrapper_t::unwrap(self, false);
      luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected an integer index");
      int isnum;
      int i = lua_tointegerx(L, 2, &isnum);
      luaL_argcheck(L, isnum, 2, "expected an integer index");
      if (!prop)
         return 0;
      auto& list = prop->values;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      self.before_edit();
      list[i].clear(*self.form);
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
      int isnum;
      int i = lua_tointegerx(L, index_key, &isnum);
      if (!isnum)
         cobb::lua::error(L, "cannot use string keys or non-integer keys");
      if (i < 1)
         cobb::lua::error(L, "indices below 1, such as %d, are not allowed", i);
      --i;
      //
      auto& self = get_collection_wrapper(L);
      auto* prop = prop_wrapper_t::unwrap(self, false);
      if (!prop)
         return 0;
      //
      if (!dovahscript::api_helpers::papyrus::property_scalar_value_typecheck(L, index_value, prop->type))
         cobb::lua::error(L, "desired value is of the wrong type for this property");
      //
      self.before_edit();
      auto& list = prop->values;
      auto  size = list.size();
      if (i >= size) {
         if (i > size) {
            cobb::lua::warning(L, "index %s is out of bounds; nil elements will be created between the end of the list and the new element", lua_tolstring(L, index_key, nullptr));
         }
         list.resize(i + 1);
      }
      dovahscript::api_helpers::papyrus::set_property_value(L, index_value, self, *prop, i);
      self.after_edit();
      return 0;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params papyrus_property_array_value_list = {
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