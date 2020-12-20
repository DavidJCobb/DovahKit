#include "form.h"
#include "../classes.h"
#include "../util.h"
#include "../../../dovah/form_stub.h"

namespace {
   using namespace editor_script;
   //
   namespace _methods {
      luastackchange_t get_form_id(lua_State* L) {
         auto* wrapper = classes::class_from_stack<classes::form>(L, 1);
         luaL_argcheck(L, wrapper != nullptr, 1, "'form' expected");
         __assume(wrapper != nullptr); // so IntelliSense doesn't warn about it maybe being null
         if (!wrapper->stub)
            return 0;
         lua_pushnumber(L, wrapper->stub->formID);
         return 1;
      }
      luastackchange_t get_editor_id(lua_State* L) {
         auto* wrapper = classes::class_from_stack<classes::form>(L, 1);
         luaL_argcheck(L, wrapper != nullptr, 1, "'form' expected");
         __assume(wrapper != nullptr); // so IntelliSense doesn't warn about it maybe being null
         if (!wrapper->stub)
            return 0;
         lua_pushstring(L, wrapper->stub->get_editor_id());
         return 1;
      }
   }
}

namespace editor_script::classes {
   /*static*/ luaL_Reg form::metatable_methods[] = {
      { "get_editor_id", &_methods::get_editor_id },
      { "get_form_id",   &_methods::get_form_id },
   };
}