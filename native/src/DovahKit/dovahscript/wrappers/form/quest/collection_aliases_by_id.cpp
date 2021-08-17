#include "collection_aliases_by_id.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../wrapper.h"

#include "../../../../dovah/forms/Quest.h"
#include "../quest.h"
#include "alias.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.quest.aliases_by_id>";
}

namespace {
   using namespace dovahscript;

   using quest_wrapper_t = dovahscript::wrappers::quest;
   using alias_wrapper_t = dovahscript::wrappers::quest_alias;
   using wrapped_type    = dovah::loaded_forms::Quest;
   
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
      uint32_t max = 0;
      for (auto* alias : form->aliases)
         if (alias && alias->id > max)
            max = alias->id;
      lua_pushinteger(L, max);
      return 1;
   }
   int lookup_item_by_name(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      auto* form = self.get_loaded_form_data<wrapped_type>();
      if (!form)
         return 0;
      int   isnum;
      auto  i     = lua_tointegerx(L, 2, &isnum);
      if (!isnum)
         return 0;
      auto* alias = form->lookup_alias_by_id(i);
      if (!alias)
         return 0;
      return alias_wrapper_t::wrap(L, self, alias);
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
         lua_rawseti(L, index_tbl, alias->id);
      }
      return 1;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params quest_alias_by_id_set = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_all_item_names     = &get_all_item_names,
      .get_collection_length  = &get_collection_length,
      .items_are_named        = true,
      .lookup_item_by_name    = &lookup_item_by_name,
   };
}