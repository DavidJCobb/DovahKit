#include "collection_aliases.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/Quest.h"
#include "../quest.h"
#include "alias.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.quest.aliases>";
}

namespace {
   using namespace dovahscript;

   using quest_wrapper_type = dovahscript::wrappers::quest;
   using alias_wrapper_type = dovahscript::wrappers::quest_alias;
   using wrapped_type       = dovah::loaded_forms::Quest;
   
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
      if (!form) {
         lua_pushinteger(L, 0);
         return 1;
      }
      lua_pushinteger(L, form->aliases.size());
      return 1;
   }
   int lookup_item_by_name(lua_State* L) {
      //
      // args: wrapper<papyrus_root>, name
      //
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      const char* name = lua_tostring(L, 2);
      if (!name)
         return 0;
      auto& list = form->aliases;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         const auto* alias = list[i];
         if (_stricmp(alias->name.c_str(), name) == 0)
            return alias_wrapper_type::wrap(L, self, alias);
      }
      return 0;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      auto  i    = lua_tointeger(L, 2);
      auto& list = form->aliases;
      if (i > list.size() || i <= 0)
         return 0;
      --i;
      return alias_wrapper_type::wrap(L, self, list[i]);
   }
   int get_all_item_names(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      auto& list = form->aliases;
      //
      lua_createtable(L, 0, list.size());
      auto index_tbl = lua_gettop(L);
      //
      for (const auto* alias : list) {
         lua_pushboolean(L, true);
         lua_setfield(L, index_tbl, alias->name.c_str());
      }
      return 1;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params quest_alias_set = {
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