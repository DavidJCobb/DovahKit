#include "topic.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../wrapper.h"

#include "../../../dovah/form_stub_addenda.h"
#include "../../../dovah/forms/Topic.h"

//
// MISSING APIS:
//  - Do All Before Repeating
//  - Priority
//  - Subtype (do we want to expose this?)
//
#include "../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_script_apis, "The Lua API for topics is incomplete.");

namespace {
   using namespace dovahscript;
   using cls          = wrappers::topic;
   using wrapped_type = cls::wrapped_type;

   namespace _getters {
      int infos(lua_State* L) {
         lua_settop(L, 1);
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub) {
            lua_newtable(L);
            return 1;
         }
         auto* addenda = self.stub->addenda;
         if (!addenda) {
            lua_newtable(L);
            return 1;
         }
         auto&  list = addenda->ordered_children.active_file;
         size_t size = list.size();
         lua_createtable(L, size, 0);
         size_t j = 0;
         for (size_t i = 0; i < size; ++i) {
            int wcount = push_native_object(list[i]);
            while (wcount--)
               lua_rawseti(L, -2, ++j);
         }
         return 1;
      }
      int parent_branch(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* parent = self.stub->get_outbound_use_with_flag(dovah::use_info_entry::flag::dialogue_branch);
         if (parent->form_type != dovah::form_type::dialogue_branch)
            return 0;
         return push_native_object(parent);
      }
      int parent_quest(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* parent = self.stub->get_outbound_use_with_flag(dovah::use_info_entry::flag::dialogue_quest);
         if (parent->form_type != dovah::form_type::quest)
            return 0;
         return push_native_object(parent);
      }
      int text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->text.c_str());
         return 1;
      }
   }
   namespace _setters {
      int text(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->text = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "infos",         &_getters::infos },
      { "parent_branch", &_getters::parent_branch },
      { "parent_quest",  &_getters::parent_quest },
      { "text",          &_getters::text },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "text", &_setters::text },
   };
}