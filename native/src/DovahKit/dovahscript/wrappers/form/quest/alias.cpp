#include "alias.h"
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../pull_native_object.h"
#include "../../../push_native_object.h"
#include "../../../wrap_native_object.h"
#include "../../../wrapper.h"

#include "../../../../dovah/form_stub.h"
#include "../../../../dovah/forms/Quest.h"
/*//
#include "../papyrus/root.h"
//*/
#include "../quest.h"
#include "loc_alias.h"
#include "ref_alias.h"

//
// MISSING APIS:
//  - Base
//     - Common flags
//

namespace {
   using namespace dovahscript;
   using cls = wrappers::quest_alias;
   
   namespace _base {
      namespace getters {
         int id(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<cls>(L);
            auto* alias = cls::unwrap(self);
            if (alias == nullptr)
               cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
            //
            lua_pushinteger(L, alias->id);
            return 1;
         }
         int name(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<cls>(L);
            auto* alias = cls::unwrap(self);
            if (alias == nullptr)
               cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
            //
            lua_pushstring(L, alias->name.c_str());
            return 1;
         }
         /*//
         int papyrus(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<cls>(L);
            auto* alias = cls::unwrap(self);
            if (alias == nullptr)
               cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
            wrapper out = self;
            out.append_part(wrapper_part_types::papyrus_root);
            return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_root::metatable_key);
         }
         //*/
         int parent(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<cls>(L);
            auto* alias = cls::unwrap(self);
            if (alias == nullptr)
               cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
            return push_native_object(&alias->owner.stub);
         }
         int type(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<cls>(L);
            auto* alias = cls::unwrap(self);
            if (alias == nullptr)
               return 0;
            switch (alias->type) {
               case dovah::loaded_forms::Alias::alias_type::location:
                  lua_pushstring(L, "location");
                  break;
               case dovah::loaded_forms::Alias::alias_type::reference:
                  lua_pushstring(L, "reference");
                  break;
               default:
                  lua_pushstring(L, "unknown");
                  break;
            }
            return 1;
         }
      }
      namespace setters {
         int name(lua_State* L) {
            core::subsystems::permissions::verify_form_write_permissions();
            //
            auto& self  = get_wrapper_for_thiscall<cls>(L);
            auto* alias = cls::unwrap(self);
            if (alias == nullptr)
               cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
            //
            self.before_edit();
            alias->name = lua_tolstring(L, 2, nullptr);
            self.after_edit();
            return 0;
         }
         int id(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<cls>(L);
            auto* alias = cls::unwrap(self);
            if (alias == nullptr)
               cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
            //
            int  isnum;
            auto id = lua_tointegerx(L, 2, &isnum);
            cobb::lua::argcheck(L, isnum,       2, "id (integer) expected");
            cobb::lua::argcheck(L, id >= 0,     2, "alias IDs cannot be negative");
            cobb::lua::argcheck(L, id < 0xFFFF, 2, "alias IDs cannot exceed 65534 without causing file format issues");
            //
            if (id == alias->id)
               return 0;
            auto* quest    = self.get_loaded_form_data<dovah::loaded_forms::Quest>();
            auto* conflict = quest->lookup_alias_by_id(id);
            if (conflict)
               cobb::lua::error(L, "alias ID %d is already in use by another alias on this quest", id);
            //
            self.before_edit();
            alias->id = id;
            if (quest->next_alias_id <= id)
               quest->next_alias_id = id + 1;
            self.after_edit();
            //
            return 0;
         }
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "id",      &_base::getters::id },
      { "name",    &_base::getters::name },
      { "parent",  &_base::getters::parent },
      /*//
      { "papyrus", &_base::getters::papyrus },
      //*/
      { "type",    &_base::getters::type },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "id",   &_base::setters::id },
      { "name", &_base::setters::name },
   };

   /*static*/ cls::wrapped_type* cls::unwrap(wrapper& w) {
      if (w.depth <= 1 && w.is_collection)
         return nullptr;
      auto* form = w.get_loaded_form_data<dovah::loaded_forms::Quest>();
      if (!form)
         return nullptr;
      auto index = w.parts[0].index;
      switch (w.parts[0].signature) {
         case wrapper_part_types::quest_alias:
            if (index >= form->aliases.size())
               return nullptr;
            return form->aliases[index];
         case wrapper_part_types::quest_alias_by_id:
            return form->lookup_alias_by_id(index);
      }
      return nullptr;
   }

   /*static*/ int cls::wrap(lua_State* L, dovah::form_stub* quest, uint32_t aliasID) {
      if (!quest)
         return 0;
      wrapper out = wrap_native_object(*quest);
      out.append_part(dovahscript::wrapper_part_types::quest_alias);
      out.is_collection = true;
      auto* loaded = out.get_loaded_form_data<dovah::loaded_forms::Quest>();
      return wrap(L, out, loaded->lookup_alias_by_id(aliasID));
   }
   /*static*/ int cls::wrap(lua_State* L, dovah::form_stub* quest, const wrapped_type* alias) {
      if (!quest || !alias)
         return 0;
      wrapper out = wrap_native_object(*quest);
      out.append_part(dovahscript::wrapper_part_types::quest_alias);
      out.is_collection = true;
      return wrap(L, out, alias);
   }
   /*static*/ int cls::wrap(lua_State* L, const wrapper& collection, const wrapped_type* alias) {
      wrapper out = collection;
      assert(out.is_collection);
      assert(out.parts[0].signature == wrapper_part_types::quest_alias_by_id || out.parts[0].signature == wrapper_part_types::quest_alias);
      out.parts[0].signature = wrapper_part_types::quest_alias_by_id; // force non-ID access to ID access
      out.into_collection(alias->id);
      out.last_part().noncontiguous = true;
      //
      const auto* metatable_key = wrappers::quest_alias::metatable_key;
      if (alias) {
         switch (alias->type) {
            case wrapped_type::alias_type::location:
               metatable_key = wrappers::quest_loc_alias::metatable_key;
               break;
            case wrapped_type::alias_type::reference:
               metatable_key = wrappers::quest_ref_alias::metatable_key;
               break;
         }
      }
      return core::subsystems::userdata::get().push(L, out, metatable_key);
   }
}