#include "wrapper.h"
#include <cassert>
#include "editor_script_core.h"
#include "util.h"
#include "wrapper_util.h"

namespace editor_script {
   /*static*/ luastackchange_t wrapper::__gc(lua_State* L) {
      auto* userdata = (wrapper*)lua_touserdata(L, 1);
      userdata->~wrapper();
      return 0;
   }
}

namespace editor_script { // base metatable
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_metatable::metatable_methods = {
      { "__gc",  &wrapper::__gc },
   };
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_metatable::metatable_getters = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_metatable::metatable_setters = no_functions;
}

namespace editor_script {
   void wrapper::append_part(part_type_t signature, uint32_t index) {
      assert(this->depth < part_count && "Too many parts!");
      auto& part = this->parts[this->depth];
      part.signature = signature;
      part.index     = index;
      ++this->depth;
   }
   void wrapper::append_part(part_type_t signature, const std::string& name) {
      assert(this->depth < part_count && "Too many parts!");
      auto& part = this->parts[this->depth];
      part.signature = signature;
      part.name      = name;
      ++this->depth;
   }
   void wrapper::append_part(part_type_t signature, const char* name) {
      assert(this->depth < part_count && "Too many parts!");
      auto& part = this->parts[this->depth];
      part.signature = signature;
      part.name      = name;
      ++this->depth;
   }
   void wrapper::remove_part() {
      --this->depth;
      auto& removed = this->parts[this->depth];
      removed.signature = 0;
      removed.index     = 0;
      removed.name.clear();
   }
   void wrapper::into_collection(uint32_t index) {
      assert(this->is_collection);
      this->is_collection = false;
      this->parts[this->depth - 1].index = index;
   }
   void wrapper::into_collection(const char* name) {
      assert(this->is_collection);
      this->is_collection = false;
      this->parts[this->depth - 1].name = name;
   }

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