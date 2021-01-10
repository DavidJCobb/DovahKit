#include "script.h"
#include "../../classes.h"
#include "../../util.h"
#include "../../../../dovah/form_stub.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../../../dovah/forms/Form.h"
#include "root.h"

namespace {
   using namespace editor_script;
   //
   namespace _getters {
      luastackchange_t name(lua_State* L) {
         auto& self   = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         __assume(script != nullptr);
         //
         lua_pushstring(L, script->name.c_str());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t name(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self   = get_wrapper_for_thiscall<wrappers::papyrus_script>(L);
         auto* script = wrappers::papyrus_script::unwrap(self);
         if (script == nullptr)
            luaL_error(L, "script wrapper has no underlying object (deleted?)");
         __assume(script != nullptr);
         //
         self.before_edit();
         script->name = lua_tolstring(L, 2, nullptr);
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_script::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_script::metatable_getters = {
      { "name", &_getters::name },
   };
   /*static*/ const std::initializer_list<luaL_Reg> papyrus_script::metatable_setters = {
      { "name", &_setters::name },
   };

   /*static*/ papyrus_script::wrapped_t* papyrus_script::unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[1].signature != cobb::eight_cc("PapyScri"))
         return nullptr;
      auto* root = papyrus_root::unwrap(w);
      if (!root)
         return nullptr;
      auto& list = root->scripts;
      auto  i    = w.parts[1].index;
      if (i >= list.size())
         return nullptr;
      return &list[i];
   }
}