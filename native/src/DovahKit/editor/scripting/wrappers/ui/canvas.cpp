#include "canvas.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::canvas;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
   }
   namespace _setters {
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.canvas.new function should not be called with a colon or passed any arguments");
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type();
            DovahKitScriptVMCore::get().set_up_new_scripted_widget(created);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         wrapper out;
         auto* mt = wrap_widget(out, created);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, cls::global_name);
   }
}