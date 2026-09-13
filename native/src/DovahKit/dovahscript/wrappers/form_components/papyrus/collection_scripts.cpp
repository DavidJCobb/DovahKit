#include "./collection_scripts.h"
#include "helpers/lua/error.h"
#include "dovahscript/api_helpers/fail_if_form_cannot_be_edited.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/wrapper.h"

#include "dovah/data/papyrus/helpers/name_equals.h"
#include "dovah/forms/components/papyrus/attachment_data.h"
#include "../papyrus.h"
#include "./script.h"
#include "dovah/forms/Form.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.papyrus.script>";
}

namespace {
   using namespace dovahscript;
   using root_wrapper = wrappers::papyrus_root;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      }
      return *self;
   }

   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* root = root_wrapper::unwrap(self);
      if (!root)
         return 0;
      lua_pushinteger(L, root->scripts.size());
      return 1;
   }
   int lookup_item_by_name(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* root = root_wrapper::unwrap(self);
      if (!root)
         return 0;

      if (!lua_isstring(L, 2))
         return 0;
      std::string_view name = lua_tostring(L, 2);
      if (name.empty())
         return 0;

      for (size_t i = 0; i < root->scripts.size(); ++i) {
         if (dovah::papyrus::helpers::name_equals(name, root->scripts[i].name)) {
            wrapper out = self;
            out.into_collection(i);
            return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_script::metatable_key);
         }
      }
      lua_pushnil(L);
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* root = root_wrapper::unwrap(self);
      if (!root)
         return 0;

      auto  i = lua_tointeger(L, 2);
      auto& list = root->scripts;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      wrapper out = self;
      out.into_collection(i);
      return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_script::metatable_key);
   }
   int get_all_item_names(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* root = root_wrapper::unwrap(self);
      if (!root)
         return 0;

      auto& list = root->scripts;
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
   int member_function_insert(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();
      
      auto& self = get_collection_wrapper(L);
      api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
      auto* root = root_wrapper::unwrap(self);
      if (!root)
         return 0;

      cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string (script name) expected");
      std::string_view scriptname = lua_tostring(L, 2);
      cobb::lua::argcheck(L, !scriptname.empty(), 2, "script name cannot be empty");
      if (root->lookup_script(scriptname)) {
         cobb::lua::argerror(L, 2, "a script with this name is already attached");
      }

      self.before_edit();
      {
         auto& script = root->scripts.emplace_back();
         script.name = scriptname;
      }
      self.after_edit();

      wrapper out = self;
      out.into_collection(root->scripts.size() - 1);
      return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_script::metatable_key);
   }
   int member_function_remove(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();

      auto& self = get_collection_wrapper(L);
      api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
      auto* root = root_wrapper::unwrap(self);
      if (!root)
         return 0;

      cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string (script name) expected");
      std::string_view scriptname = lua_tostring(L, 2);

      std::optional<size_t> index;
      for (size_t i = 0; i < root->scripts.size(); ++i) {
         if (dovah::papyrus::helpers::name_equals(scriptname, root->scripts[i].name)) {
            index = i;
            break;
         }
      }
      if (!index.has_value())
         return 0;

      auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
      self.before_edit();
      root->scripts[index.value()].clear(*form);
      root->scripts.erase(root->scripts.begin() + index.value());
      self.after_edit();
      {
         wrapper to_remove = self;
         to_remove.into_collection(index.value());
         core::subsystems::userdata::get().remove_from_sequential_collection(to_remove);
      }
      return 0;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params papyrus_scripts = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_all_item_names     = &get_all_item_names,
      .get_collection_length  = &get_collection_length,
      .items_are_named        = true,
      .lookup_item_by_name    = &lookup_item_by_name,
      .lookup_item_by_index   = &lookup_item_by_index,
      .member_function_insert = &member_function_insert,
      .member_function_remove = &member_function_remove,
   };
}