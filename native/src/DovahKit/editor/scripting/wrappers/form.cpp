#include "form.h"
#include "../classes.h"
#include "../util.h"
#include "../../../dovah/forms/Form.h"
#include "../editor_script_core.h"
#include "../wrapper_util.h"

#include "../messages/delete_form.h"

#include "papyrus/root.h"

namespace {
   using namespace editor_script;
   //
   namespace _methods {
      luastackchange_t delete_(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrappers::form>(L);
         if (!self.stub)
            return 0;
         auto* m = new messages::delete_form;
         m->stub = self.stub;
         DovahKitScriptVMMessenger::get().send_message(m);
         return 0;
      }
      luastackchange_t get_editor_id(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::form>(L);
         if (!self.stub)
            return 0;
         lua_pushstring(L, self.stub->get_editor_id());
         return 1;
      }
      luastackchange_t get_form_id(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::form>(L);
         if (!self.stub)
            return 0;
         lua_pushnumber(L, self.stub->formID);
         return 1;
      }
      luastackchange_t get_form_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::form>(L);
         if (!self.stub)
            return 0;
         lua_pushnumber(L, self.stub->formType);
         return 1;
      }
      luastackchange_t get_user_forms(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::form>(L);
         if (!self.stub)
            return 0;
         //
         auto* stub  = self.stub;
         lua_createtable(L, stub->inbound.size(), 0);
         auto  table = lua_gettop(L);
         //
         auto& vm = DovahKitScriptVMUserdataInterface::get();
         int   i  = 1; // Lua arrays start with 1, remember?
         for (auto& pair : stub->inbound) {
            auto& entry = pair.second;
            if (!entry.other)
               continue;
            wrapper out;
            auto*   mt = wrap_form(out, stub);
            if (0 == DovahKitScriptVMUserdataInterface::get().push(L, out, mt))
               continue;
            lua_seti(L, table, i);
            ++i;
         }
         //
         return 1;
      }
   }
   namespace _getters {
      luastackchange_t papyrus(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::form>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         if (!form)
            return 0;
         auto* root = form->get_papyrus_data();
         if (!root)
            return 0; // this form type can't have Papyrus data (or loading it isn't implemented yet)
         wrapper out;
         wrap_form(out, self.stub);
         out.append_part(wrapper_part_types::papyrus_root);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::papyrus_root::metatable_key);
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> form::metatable_methods = {
      { "delete",         &_methods::delete_ },
      { "get_editor_id",  &_methods::get_editor_id },
      { "get_form_id",    &_methods::get_form_id },
      { "get_form_type",  &_methods::get_form_type },
      { "get_user_forms", &_methods::get_user_forms },
   };
   /*static*/ const std::initializer_list<luaL_Reg> form::metatable_getters = {
      { "papyrus", &_getters::papyrus },
   };
   /*static*/ const std::initializer_list<luaL_Reg> form::metatable_setters = no_functions;
}