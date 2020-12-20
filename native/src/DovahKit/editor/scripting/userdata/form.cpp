#include "form.h"
#include "../classes.h"
#include "../util.h"
#include "../../../dovah/form_stub.h"
#include "../editor_script_core.h"

namespace {
   using namespace editor_script;
   //
   namespace _methods {
      luastackchange_t get_editor_id(lua_State* L) {
         auto* wrapper = classes::class_from_stack<classes::form>(L, 1);
         luaL_argcheck(L, wrapper != nullptr, 1, "'form' expected");
         __assume(wrapper != nullptr); // so IntelliSense doesn't warn about it maybe being null
         if (!wrapper->stub)
            return 0;
         lua_pushstring(L, wrapper->stub->get_editor_id());
         return 1;
      }
      luastackchange_t get_form_id(lua_State* L) {
         auto* wrapper = classes::class_from_stack<classes::form>(L, 1);
         luaL_argcheck(L, wrapper != nullptr, 1, "'form' expected");
         __assume(wrapper != nullptr); // so IntelliSense doesn't warn about it maybe being null
         if (!wrapper->stub)
            return 0;
         lua_pushnumber(L, wrapper->stub->formID);
         return 1;
      }
      luastackchange_t get_form_type(lua_State* L) {
         auto* wrapper = classes::class_from_stack<classes::form>(L, 1);
         luaL_argcheck(L, wrapper != nullptr, 1, "'form' expected");
         __assume(wrapper != nullptr); // so IntelliSense doesn't warn about it maybe being null
         if (!wrapper->stub)
            return 0;
         lua_pushnumber(L, wrapper->stub->formType);
         return 1;
      }
      luastackchange_t get_user_forms(lua_State* L) {
         auto* wrapper = classes::class_from_stack<classes::form>(L, 1);
         luaL_argcheck(L, wrapper != nullptr, 1, "'form' expected");
         __assume(wrapper != nullptr); // so IntelliSense doesn't warn about it maybe being null
         if (!wrapper->stub)
            return 0;
         //
         auto* stub  = wrapper->stub;
         lua_createtable(L, stub->inbound.size(), 0);
         auto  table = lua_gettop(L);
         //
         auto& vm = DovahKitScriptVMUserdataInterface::get();
         int   i  = 1; // Lua arrays start with 1, remember?
         for (auto& pair : stub->inbound) {
            auto& entry = pair.second;
            if (!entry.other)
               continue;
            classes::_base* user = new classes::form(stub);
            if (0 == vm.return_wrapper_to_lua(L, user, classes::form::metatable_key))
               continue;
            lua_seti(L, table, i);
            ++i;
         }
         //
         return 1;
      }
   }
}

namespace editor_script::classes {
   /*static*/ luaL_Reg form::metatable_methods[] = {
      { "get_editor_id",  &_methods::get_editor_id },
      { "get_form_id",    &_methods::get_form_id },
      { "get_form_type",  &_methods::get_form_type },
      { "get_user_forms", &_methods::get_user_forms },
   };
}