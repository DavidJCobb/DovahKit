#include "./dialogue_branch.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"

#include "dovahscript/wrapper.h"
#include "dovahscript/api_helpers/fail_if_form_cannot_be_edited.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/core/collections.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/lua_libraries/form_types.h"

#include "dovah/forms/DialogueBranch.h"

#include "../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_script_apis, "The Lua API for dialogue_branchs is incomplete.");

#include "dovah/form_stubs/helpers/for_each_dialogue_branch_topic.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_branch.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::dialogue_branch;
   using wrapped_type = cls::wrapped_type;
}

#pragma region form
namespace {
   namespace _methods {
      int get_all_topics(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub) {
            lua_createtable(L, 0, 0);
            return 1;
         }

         size_t expected = 0;
         dovah::form_stub_helpers::for_each_dialogue_branch_topic(*stub, [L, &expected](dovah::form_stub& topic) {
            ++expected;
         });

         lua_createtable(L, expected, 0);
         int i   = 0;
         int pos = lua_gettop(L);
         dovah::form_stub_helpers::for_each_dialogue_branch_topic(*stub, [L, &i, &pos](dovah::form_stub& topic) {
            int wcount = push_native_object(&topic);
            while (wcount--)
               lua_rawseti(L, pos, ++i);
         });
         assert(lua_gettop(L) == pos);
         return 1;
      }
   }
   namespace _getters {
      int exclusive(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, form->branch_flags & wrapped_type::branch_flag::exclusive);
         return 1;
      }
      int parent_quest(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->owning_quest);
      }
      int starting_topic(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->starting_topic);
      }
      int type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         switch (form->branch_flags & 0b11) {
            case wrapped_type::branch_flag::normal:
               lua_pushstring(L, "normal");
               break;
            case wrapped_type::branch_flag::top_level:
               lua_pushstring(L, "top-level");
               break;
            case wrapped_type::branch_flag::blocking:
               lua_pushstring(L, "blocking");
               break;
            default:
               lua_pushnil(L);
               break;
         }
         return 1;
      }
   }
   namespace _setters {
      int exclusive(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "expected boolean");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->branch_flags, wrapped_type::branch_flag::exclusive, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      int starting_topic(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::topic);
         if (!value)
            cobb::lua::argerror(L, 2, "you must specify a topic");
         if (!form)
            return 0;
         {
            auto* owning_branch = dovah::form_stub_helpers::get_dialogue_topic_branch(*value);
            if (owning_branch != self.stub) {
               cobb::lua::argerror(L, 2, "the specified topic does not belong to this branch");
            }
         }
         self.before_edit();
         form->starting_topic.set(*form, value);
         self.after_edit();
         return 0;
      }
      int type(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         
         uint32_t raw = 0;

         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "expected string");
         {
            std::string_view v = lua_tostring(L, 2);
            if (v == "normal")
               raw = wrapped_type::branch_flag::normal;
            else if (v == "top-level")
               raw = wrapped_type::branch_flag::top_level;
            else if (v == "blocking")
               raw = wrapped_type::branch_flag::blocking;
            else
               cobb::lua::argerror(L, 2, "unrecognized type");
         }
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;

         self.before_edit();
         form->branch_flags &= ~(0b11);
         form->branch_flags |= raw;
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "get_all_topics", &_methods::get_all_topics },
   };
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "exclusive",      &_getters::exclusive },
      { "parent_quest",   &_getters::parent_quest },
      { "starting_topic", &_getters::starting_topic },
      { "type",           &_getters::type },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "exclusive",      &_setters::exclusive },
      { "starting_topic", &_setters::starting_topic },
      { "type",           &_setters::type },
   };
}
#pragma endregion