#include "./collection_properties.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/wrapper.h"

#include "dovah/data/papyrus/helpers/name_equals.h"
#include "dovah/forms/components/papyrus/attachment_data.h"
#include "../papyrus.h"
#include "./property.h"
#include "./script.h"
#include "dovah/forms/Form.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.papyrus.property>";
}

namespace {
   using namespace dovahscript;
   using parent_wrapper = wrappers::papyrus_script;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      }
      return *self;
   }

   int get_collection_length(lua_State* L) {
      auto& self   = get_collection_wrapper(L);
      auto* script = parent_wrapper::unwrap(self);
      if (!script)
         return 0;
      lua_pushinteger(L, script->properties.size());
      return 1;
   }
   int lookup_item_by_name(lua_State* L) {
      auto& self   = get_collection_wrapper(L);
      auto* script = parent_wrapper::unwrap(self);
      if (!script)
         return 0;

      if (!lua_isstring(L, 2))
         return 0;
      std::string_view name = lua_tostring(L, 2);
      if (name.empty())
         return 0;

      for (size_t i = 0; i < script->properties.size(); ++i) {
         if (dovah::papyrus::helpers::name_equals(name, script->properties[i].name)) {
            wrapper out = self;
            out.into_collection(i);
            return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_property::metatable_key);
         }
      }
      lua_pushnil(L);
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self   = get_collection_wrapper(L);
      auto* script = parent_wrapper::unwrap(self);
      if (!script)
         return 0;

      auto  i = lua_tointeger(L, 2);
      auto& list = script->properties;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      wrapper out = self;
      out.into_collection(i);
      return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_property::metatable_key);
   }
   int get_all_item_names(lua_State* L) {
      auto& self   = get_collection_wrapper(L);
      auto* script = parent_wrapper::unwrap(self);
      if (!script)
         return 0;

      auto& list = script->properties;
      //
      lua_createtable(L, 0, list.size());
      auto index_tbl = lua_gettop(L);
      //
      for (const auto& script : list) {
         lua_pushboolean(L, true);
         lua_setfield(L, index_tbl, script.name.c_str());
      }
      return 1;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params papyrus_properties = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_all_item_names     = &get_all_item_names,
      .get_collection_length  = &get_collection_length,
      .items_are_named        = true,
      .lookup_item_by_name    = &lookup_item_by_name,
      .lookup_item_by_index   = &lookup_item_by_index,
   };
}