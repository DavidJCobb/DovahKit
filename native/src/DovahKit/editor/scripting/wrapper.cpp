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
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_metatable::metatable_methods = {
      { "__gc",  &_methods::__gc },
      { nullptr, nullptr },
   };
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_metatable::metatable_getters = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_metatable::metatable_setters = no_functions;
}

namespace editor_script {
   bool wrapper::part::operator==(const part& other) const noexcept {
      if (this->signature != other.signature)
         return false;
      if (this->name.empty()) {
         if (!other.name.empty())
            return false;
         if (this->index != other.index)
            return false;
      } else {
         if (other.name.empty())
            return false;
         if (this->name != other.name)
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
      if (this->is_collection) {
         uint8_t i = 0;
         for (; i < (signed int)(this->depth) - 1; ++i)
            if (this->parts[i] != other->parts[i])
               return false;
         if (this->parts[i].signature != other->parts[i].signature) // for collections, the last part has no index
            return false;
      } else {
         for (uint8_t i = 0; i < this->depth; ++i)
            if (this->parts[i] != other->parts[i])
               return false;
      }
      return true;
   }
   void wrapper::load_form() {
      if (!this->stub)
         return;
      this->form = this->stub->load();
   }
   void wrapper::mark_form_as_edited() {
      if (!this->stub)
         return;
      this->stub->set_edited(true);
   }
}