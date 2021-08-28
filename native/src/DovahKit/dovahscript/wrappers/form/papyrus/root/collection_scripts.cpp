#include "collection_scripts.h"
#include "../../../../../helpers/lua/error.h"
#include "../../../../core/subsystems/permissions.h"
#include "../../../../core/subsystems/userdata.h"
#include "../../../../core/classes.h"
#include "../../../../wrapper.h"

#include "../root.h"
#include "../script.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.papyrus_root.scripts>";
}

namespace {
   using namespace dovahscript;

   using root_wrapper_type   = dovahscript::wrappers::papyrus_root;
   using script_wrapper_type = dovahscript::wrappers::papyrus_script;

   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      }
      return *self;
   }

   int lookup_item_by_name(lua_State* L) {
      //
      // args: wrapper<papyrus_root>, name
      //
      auto& self = get_collection_wrapper(L);
      auto* root = root_wrapper_type::unwrap(self, false);
      if (!root)
         luaL_error(L, "wrapper `%s` has no underlying object (deleted?)", collection_metatable_key);
      const char* name = lua_tostring(L, 2);
      if (!name)
         return 0;
      auto& list = root->scripts;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& script = list[i];
         if (stricmp(script.name.c_str(), name) == 0) {
            wrapper out = self;
            assert(out.is_collection);
            out.into_collection(i);
            return core::subsystems::userdata::get().push(L, out, collection_metatable_key);
         }
      }
      return 0;
   }
   int get_all_item_names(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* root = root_wrapper_type::unwrap(self, false);
      if (!root)
         cobb::lua::error(L, "wrapper `%s` has no underlying object (deleted?)", collection_metatable_key);
      auto& list = root->scripts;
      //
      lua_createtable(L, 0, list.size());
      auto index_tbl = lua_gettop(L);
      //
      for (auto& script : list) {
         lua_pushboolean(L, true);
         lua_setfield(L, index_tbl, script.name.c_str());
      }
      return 1;
   }
   int set_item(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      //
      constexpr auto index_self  = 1;
      constexpr auto index_key   = 2;
      constexpr auto index_value = 3;
      //
      auto& self = get_collection_wrapper(L);
      auto* root = root_wrapper_type::unwrap(self, false);
      if (!root)
         cobb::lua::error(L, "wrapper `%s` has no underlying object (deleted?)", collection_metatable_key);
      if (!lua_isstring(L, index_key))
         cobb::lua::error(L, "the given key is not auto-convertible to a string and thus cannot be a Papyrus script name");
      std::string name = lua_tostring(L, index_key);
      //
      if (lua_isnoneornil(L, index_value)) {
         //
         // Setting a script to nil should remove it.
         //
         auto& list = root->scripts;
         for (auto it = list.begin(); it != list.end(); ++it) {
            auto& script = *it;
            if (_stricmp(script.name.c_str(), name.c_str()) == 0) {
               self.before_edit();
               script.clear(*self.form);
               {
                  wrapper sw = self;
                  assert(sw.is_collection);
                  sw.into_collection(it - list.begin());
                  core::subsystems::userdata::get().remove_from_sequential_collection(sw);
               }
               list.erase(it);
               self.after_edit();
               break;
            }
         }
      } else {
         auto* arg = (wrapper*) classes::cast_to_class(L, index_value, script_wrapper_type::metatable_key);
         if (!arg)
            cobb::lua::error(L, "you can only overwrite a Papyrus script with nil or with another Papyrus script");
         auto* source = script_wrapper_type::unwrap(*arg, true);
         if (source == nullptr)
            cobb::lua::error(L, "the script wrapper provided as a value to set has no underlying object (deleted?)");
         //
         self.before_edit();
         if (auto* prior = root->lookup_script(name)) {
            name = prior->name; // preserve case
            prior->clear(*self.form);
            prior->clone_from(*source, *self.form);
            prior->name = name; // restore name (it may have been changed during the clone operation)
         } else {
            auto& added = root->scripts.emplace_back();
            added.clone_from(*source, *self.form);
            added.name = name; // set name after the cloning operation
         }
         self.after_edit();
      }
      return 0;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params papyrus_script_list = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_all_item_names     = &get_all_item_names,
      .items_are_named        = true,
      .lookup_item_by_name    = &lookup_item_by_name,
      .set_item               = &set_item,
   };
}