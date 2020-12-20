#include "_base.h"
#include <cassert>
#include "../editor_script_core.h"
#include "../classes.h"
#include "../util.h"
#include "../../../dovah/form_stub.h"

namespace {
   using namespace editor_script;
   //
   namespace _methods {
      luastackchange_t __gc(lua_State* L) {
         auto* wrapper = classes::class_from_stack<classes::_base>(L, 1);
         if (!wrapper)
            return 0;
         assert(wrapper->refcount != 0 && "Attempting to GC a userdata that already has a zero refcount!");
         if (--wrapper->refcount == 0)
            delete wrapper;
         return 0;
      }
   }
}

namespace editor_script::classes {
   /*static*/ luaL_Reg _base::metatable_methods[] = {
      { "__gc", &_methods::__gc },
   };

   _base::_base() {
      DovahKitScriptVMUserdataInterface::get().insert(this);
   }
   _base::~_base() {
      DovahKitScriptVMUserdataInterface::get().remove(this);
   }

   bool _base::is_equal(const _base* other) const noexcept {
      if (this->type != other->type)
         return false;
      if (this->type == userdata_base_type::form_data) {
         if (this->stub != other->stub)
            return false;
      }
      return true;
   }
}