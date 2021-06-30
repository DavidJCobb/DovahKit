#include "alias.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../util.h"
#include "../../../../dovah/form_stub.h"
#include "../../wrapper_util.h"

#include "../../../../dovah/forms/Quest.h"
#include "../quest.h"
#include "../papyrus/root.h"

//
// MISSING APIS:
//  - Base
//     - Common flags
//  - Location aliases
//     - Fill type and parameters
//     - Flags
//  - Reference aliases
//     - Fill type and parameters
//     - Flags
//     - Added Factions
//     - Added Inventory
//     - Added Keywords
//     - Added Packages
//     - Added Spells
//     - Package override lists
//     - Additional voicetypes
//
#ifndef _DEBUG
   #pragma message("WARNING: Are you compiling in Release? The Lua API for location aliases is incomplete!")
   #pragma message("WARNING: Are you compiling in Release? The Lua API for reference aliases is incomplete!")
#endif

namespace {
   using namespace editor_script;
   //
   namespace _base {
      namespace getters {
         luastackchange_t id(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
            auto* alias = wrappers::quest_alias::unwrap(self);
            if (alias == nullptr)
               luaL_error(L, "alias wrapper has no underlying object (deleted?)");
            __assume(alias != nullptr);
            //
            lua_pushinteger(L, alias->id);
            return 1;
         }
         luastackchange_t name(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
            auto* alias = wrappers::quest_alias::unwrap(self);
            if (alias == nullptr)
               luaL_error(L, "alias wrapper has no underlying object (deleted?)");
            __assume(alias != nullptr);
            //
            lua_pushstring(L, alias->name.c_str());
            return 1;
         }
         luastackchange_t papyrus(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
            auto* alias = wrappers::quest_alias::unwrap(self);
            if (alias == nullptr)
               luaL_error(L, "alias wrapper has no underlying object (deleted?)");
            __assume(alias != nullptr);
            wrapper out = self;
            out.append_part(wrapper_part_types::papyrus_root);
            return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_root::metatable_key);
         }
         luastackchange_t parent(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
            auto* alias = wrappers::quest_alias::unwrap(self);
            if (alias == nullptr)
               luaL_error(L, "alias wrapper has no underlying object (deleted?)");
            __assume(alias != nullptr);
            return wrap_and_push_form(L, &alias->owner.stub);
         }
         luastackchange_t type(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
            auto* alias = wrappers::quest_alias::unwrap(self);
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
         luastackchange_t name(lua_State* L) {
            DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
            //
            auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
            auto* alias = wrappers::quest_alias::unwrap(self);
            if (alias == nullptr)
               luaL_error(L, "alias wrapper has no underlying object (deleted?)");
            __assume(alias != nullptr);
            //
            self.before_edit();
            alias->name = lua_tolstring(L, 2, nullptr);
            self.after_edit();
            return 0;
         }
         luastackchange_t id(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
            auto* alias = wrappers::quest_alias::unwrap(self);
            if (alias == nullptr)
               luaL_error(L, "alias wrapper has no underlying object (deleted?)");
            __assume(alias != nullptr);
            //
            int  isnum;
            auto id = lua_tointegerx(L, 2, &isnum);
            luaL_argcheck(L, isnum,       2, "id (integer) expected");
            luaL_argcheck(L, id >= 0,     2, "alias IDs cannot be negative");
            luaL_argcheck(L, id < 0xFFFF, 2, "alias IDs cannot exceed 65534 without causing file format issues");
            //
            if (id == alias->id)
               return 0;
            auto* quest    = self.get_loaded_form_data<dovah::loaded_forms::Quest>();
            auto* conflict = quest->lookup_alias_by_id(id);
            if (conflict)
               luaL_error(L, "alias ID %d is already in use by another alias on this quest", id);
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
   }//
   namespace _loc {
      namespace getters {
      }
      namespace setters {
      }
   }
   namespace _ref {
      namespace getters {
         luastackchange_t display_name(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
            auto* alias = wrappers::quest_ref_alias::unwrap(self);
            if (alias == nullptr)
               luaL_error(L, "alias wrapper has no underlying object (deleted?)");
            __assume(alias != nullptr);
            wrapper out;
            auto* mt = wrap_form(out, alias->display_name.get_form_stub());
            return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
         }
      }
      namespace setters {
         luastackchange_t display_name(lua_State* L) {
            DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
            //
            auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
            auto* alias = wrappers::quest_ref_alias::unwrap(self);
            if (alias == nullptr)
               luaL_error(L, "alias wrapper has no underlying object (deleted?)");
            __assume(alias != nullptr);
            auto* value = pull_form_stub_argument(L, 2, dovah::form_type::message);
            self.before_edit();
            alias->display_name.set(*self.stub->form, value);
            self.after_edit();
            return 0;
         }
      }
   }
}

namespace editor_script::wrappers {
   #pragma region Quest alias base members
   /*static*/ const std::initializer_list<luaL_Reg> quest_alias::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> quest_alias::metatable_getters = {
      { "id",      &_base::getters::id },
      { "name",    &_base::getters::name },
      { "parent",  &_base::getters::parent },
      { "papyrus", &_base::getters::papyrus },
      { "type",    &_base::getters::type },
   };
   /*static*/ const std::initializer_list<luaL_Reg> quest_alias::metatable_setters = {
      { "id",   &_base::setters::id },
      { "name", &_base::setters::name },
   };
   #pragma endregion

   #pragma region Location alias members
   /*static*/ const std::initializer_list<luaL_Reg> quest_loc_alias::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> quest_loc_alias::metatable_getters = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> quest_loc_alias::metatable_setters = no_functions;
   #pragma endregion

   #pragma region Reference alias members
   /*static*/ const std::initializer_list<luaL_Reg> quest_ref_alias::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> quest_ref_alias::metatable_getters = {
      { "display_name", &_ref::getters::display_name },
   };
   /*static*/ const std::initializer_list<luaL_Reg> quest_ref_alias::metatable_setters = {
      { "display_name", &_ref::setters::display_name },
   };
   #pragma endregion

   /*static*/ quest_alias::wrapped_t* quest_alias::unwrap(wrapper& w) {
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
   /*static*/ quest_loc_alias::wrapped_t* quest_loc_alias::unwrap(wrapper& w) {
      auto* alias = quest_alias::unwrap(w);
      if (alias && alias->type != dovah::loaded_forms::Alias::alias_type::location)
         return nullptr;
      return (wrapped_t*)alias;
   }
   /*static*/ quest_ref_alias::wrapped_t* quest_ref_alias::unwrap(wrapper& w) {
      auto* alias = quest_alias::unwrap(w);
      if (alias && alias->type != dovah::loaded_forms::Alias::alias_type::reference)
         return nullptr;
      return (wrapped_t*)alias;
   }

   /*static*/ luastackchange_t quest_alias::wrap(lua_State* L, dovah::form_stub* quest, uint32_t aliasID) {
      if (!quest)
         return 0;
      wrapper out;
      wrap_form(out, quest);
      out.append_part(editor_script::wrapper_part_types::quest_alias);
      out.is_collection = true;
      auto* loaded = out.get_loaded_form_data<dovah::loaded_forms::Quest>();
      return wrap(L, out, loaded->lookup_alias_by_id(aliasID));
   }
   /*static*/ luastackchange_t quest_alias::wrap(lua_State* L, dovah::form_stub* quest, const wrapped_t* alias) {
      if (!quest || !alias)
         return 0;
      wrapper out;
      wrap_form(out, quest);
      out.append_part(editor_script::wrapper_part_types::quest_alias);
      out.is_collection = true;
      return wrap(L, out, alias);
   }
   /*static*/ luastackchange_t quest_alias::wrap(lua_State* L, const wrapper& collection, const wrapped_t* alias) {
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
            case wrapped_t::alias_type::location:
               metatable_key = wrappers::quest_loc_alias::metatable_key;
               break;
            case wrapped_t::alias_type::reference:
               metatable_key = wrappers::quest_ref_alias::metatable_key;
               break;
         }
      }
      return DovahKitScriptVMUserdataInterface::get().push(L, out, metatable_key);
   }
}