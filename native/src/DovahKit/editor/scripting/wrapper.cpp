#include "wrapper.h"
#include <cassert>
#include "editor_script_core.h"
#include "util.h"
#include "wrapper_util.h"

namespace editor_script { // base metatable
   namespace {
      namespace _methods {
         luastackchange_t __gc(lua_State* L) {
            auto* userdata = (wrapper*) lua_touserdata(L, 1);
            userdata->~wrapper();
            return 0;
         }
      }
   }
   /*static*/ luaL_Reg wrapper_metatable::metatable_methods[] = {
      { "__gc",  &_methods::__gc },
      { nullptr, nullptr },
   };
}

namespace editor_script {
   bool wrapper::part::operator==(const part& other) const noexcept {
      if (this->signature != other.signature)
         return false;
      if (part_type_uses_name_key(this->signature)) {
         if (strcmp(this->name, other.name) != 0)
            return false;
      } else {
         if (this->index != other.index)
            return false;
      }
      return true;
   }

   bool wrapper::is_equal(const wrapper* other) const noexcept {
      if (this->type != other->type)
         return false;
      if (this->type == wrapper_type::form_data) {
         if (this->stub != other->stub)
            return false;
      }
      if (this->depth != other->depth)
         return false;
      if (this->is_collection != other->is_collection)
         return false;
      for (uint8_t i = 0; i < this->depth; ++i)
         if (this->parts[i] != other->parts[i])
            return false;
      return true;
   }
   void wrapper::load_form() {
      if (!this->stub)
         return;
      this->form = this->stub->load();
   }
}