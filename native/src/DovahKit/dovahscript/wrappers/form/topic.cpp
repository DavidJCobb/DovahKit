#include "topic.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../wrapper.h"

#include "../../../dovah/data/dialogue/topic_subtype.h"
#include "../../../dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "../../../dovah/use_info/entry_flags/topic.h"
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
      int do_all_before_repeating(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, !!(form->data.flags & wrapped_type::dialogue_flag::do_all_before_repeating));
         return 1;
      }
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
         auto* parent = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info::entry_flags::topic::parent_branch>(*self.stub);
         if (parent->form_type != dovah::form_type::dialogue_branch)
            return 0;
         return push_native_object(parent);
      }
      int parent_quest(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* parent = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info::entry_flags::topic::parent_quest>(*self.stub);
         if (parent->form_type != dovah::form_type::quest)
            return 0;
         return push_native_object(parent);
      }
      int priority(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->priority);
         return 1;
      }
      int subtype(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         auto signature = form->subtype;
         if (signature == 0) {
            lua_pushnil(L);
            return 1;
         }
         auto* subtype = dovah::dialogue::topic_subtype_by_signature(form->subtype);
         if (!subtype) {
            lua_pushnil(L);
            return 1;
         }
         lua_pushstring(L, subtype->internal_name.data());
         return 1;
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
      int do_all_before_repeating(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "expected boolean");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->data.flags, wrapped_type::dialogue_flag::do_all_before_repeating, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      int priority(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected number");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->priority = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int subtype(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         
         const dovah::dialogue::topic_subtype* subtype = nullptr;
         size_t subtype_index = 0;

         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         {
            const char* param = lua_tostring(L, 2);
            const auto& list  = dovah::dialogue::all_topic_subtypes;
            for (size_t i = 0; i < list.size(); ++i) {
               const auto& s = list[i];
               if (s.internal_name == param) {
                  subtype       = &s;
                  subtype_index = i;
                  break;
               }
            }
            luaL_argcheck(L, subtype != nullptr, 2, "unrecognized subtype name");
         }
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->subtype = subtype->signature;
         form->data.subtype = subtype_index;
         self.after_edit();
         return 0;
      }
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
      { "do_all_before_repeating", &_getters::do_all_before_repeating },
      { "infos",         &_getters::infos },
      { "parent_branch", &_getters::parent_branch },
      { "parent_quest",  &_getters::parent_quest },
      { "priority",      &_getters::priority },
      { "subtype",       &_getters::subtype },
      { "text",          &_getters::text },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "do_all_before_repeating", &_setters::do_all_before_repeating },
      { "priority",      &_setters::priority },
      { "subtype",       &_setters::subtype },
      { "text",          &_setters::text },
   };
}