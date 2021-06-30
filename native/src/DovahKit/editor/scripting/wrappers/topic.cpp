#include "topic.h"
#include "../systems/messaging.h"
#include "../systems/permissions.h"
#include "../systems/userdata.h"

#include "../classes.h"
#include "../util.h"
#include "../wrapper_util.h"

#include "../../../dovah/form_stub_addenda.h"

//
// MISSING APIS:
//  - Do All Before Repeating
//  - Priority
//  - Subtype (do we want to expose this?)
//
#ifndef _DEBUG
   #pragma message("WARNING: Are you compiling in Release? The Lua API for topics is incomplete!")
#endif

namespace {
   using namespace editor_script;
   using _wrapper_t     = wrappers::topic;
   using _loaded_form_t = dovah::loaded_forms::Topic;
}

#pragma region form
namespace {
   namespace _getters {
      luastackchange_t infos(lua_State* L) {
         lua_settop(L, 1);
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         if (!self.stub) {
            lua_newtable(L);
            return 1;
         }
         auto* addenda = self.stub->addenda;
         if (!addenda) {
            lua_newtable(L);
            return 1;
         }
         auto&  list = addenda->ordered_children;
         size_t size = list.size();
         lua_createtable(L, size, 0);
         size_t j = 0;
         for (size_t i = 0; i < size; ++i) {
            int wcount = wrap_and_push_form(L, list[i]);
            while (wcount--)
               lua_rawseti(L, -2, ++j);
         }
         return 1;
      }
      luastackchange_t parent_branch(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         if (!self.stub)
            return 0;
         auto* parent = self.stub->get_outbound_use_with_flag(dovah::use_info_entry::flag::dialogue_branch);
         if (parent->formType != dovah::form_type::dialogue_branch)
            return 0;
         return wrap_and_push_form(L, parent);
      }
      luastackchange_t parent_quest(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         if (!self.stub)
            return 0;
         auto* parent = self.stub->get_outbound_use_with_flag(dovah::use_info_entry::flag::dialogue_quest);
         if (parent->formType != dovah::form_type::quest)
            return 0;
         return wrap_and_push_form(L, parent);
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->text.c_str());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t text(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->text = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_methods = no_functions;
   
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_getters = {
      { "infos",         &_getters::infos },
      { "parent_branch", &_getters::parent_branch },
      { "parent_quest",  &_getters::parent_quest },
      { "text",          &_getters::text },
   };
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_setters = {
      { "text", &_setters::text },
   };
}
#pragma endregion