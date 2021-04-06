#include "alias.h"
#include "../../classes.h"
#include "../../util.h"
#include "../../../../dovah/form_stub.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../../../dovah/forms/Quest.h"
#include "../quest.h"

namespace {
   using namespace editor_script;
   //
   namespace _base {//
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
         luastackchange_t parent(lua_State* L) {
            auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
            auto* alias = wrappers::quest_alias::unwrap(self);
            if (alias == nullptr)
               luaL_error(L, "alias wrapper has no underlying object (deleted?)");
            __assume(alias != nullptr);
            wrapper out;
            auto* mt = wrap_form(out, &alias->owner.stub);
            return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
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
            auto* other = wrapper_from_stack<wrappers::form>(L, 2);
            other->error_if_wrong_form_type(L, 2, dovah::form_type::message, true);
            self.before_edit();
            alias->display_name.set(*self.stub->form, other->stub);
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
      { "id",     &_base::getters::id },
      { "name",   &_base::getters::name },
      { "parent", &_base::getters::parent },
      { "type",   &_base::getters::type },
   };
   /*static*/ const std::initializer_list<luaL_Reg> quest_alias::metatable_setters = {
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
      if (w.is_collection)
         return nullptr;
      auto* form = w.get_loaded_form_data<dovah::loaded_forms::Quest>();
      if (!form)
         return nullptr;
      auto index = w.parts[0].index;
      switch (w.parts[0].signature) {
         case cobb::eight_cc("QstAlias"):
            if (index >= form->aliases.size())
               return nullptr;
            return form->aliases[index];
         case cobb::eight_cc("QstAlsID"):
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
      assert(out.parts[0].signature == cobb::eight_cc("QstAlsID") || out.parts[0].signature == cobb::eight_cc("QstAlias"));
      out.parts[0].signature = cobb::eight_cc("QstAlsID"); // force non-ID access to ID access
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