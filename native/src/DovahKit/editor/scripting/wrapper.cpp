#include "wrapper.h"
#include <cassert>
#include "editor_script_core.h"
#include "util.h"
#include "wrapper_util.h"
#include "../core.h"

namespace editor_script {
   /*static*/ luastackchange_t wrapper::__gc(lua_State* L) {
      auto* userdata = (wrapper*)lua_touserdata(L, 1);
      userdata->teardown();
      userdata->~wrapper();
      lua_pushnil(L);
      lua_setmetatable(L, 1); // Lua can't guarantee that __gc will only be called once, so make sure there *is* no __gc to call a second time
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
   wrapper::~wrapper() {
   }
   void wrapper::teardown() {
      //
      // Flag widgets as unreferenced when they are.
      //
      if (this->type == wrapper_type::ui) {
         auto& vm = DovahKitScriptVM::get();
         vm.widget_no_longer_referenced(this->widget);
         vm.model_observer_reference_lost(this->model_observer);
         this->widget         = nullptr;
         this->model_observer = nullptr;
      }
   }

   void wrapper::append_part(part_type_t signature, uint32_t index) {
      assert(this->depth < part_count && "Too many parts!");
      auto& part = this->parts[this->depth];
      part.signature = signature;
      part.index     = index;
      ++this->depth;
   }
   void wrapper::remove_part() {
      --this->depth;
      auto& removed = this->parts[this->depth];
      removed.signature = 0;
      removed.index     = 0;
   }
   void wrapper::into_collection(uint32_t index) {
      assert(this->is_collection);
      this->is_collection = false;
      this->parts[this->depth - 1].index = index;
   }

   bool wrapper::part::operator==(const part& other) const noexcept {
      if (this->signature != other.signature)
         return false;
      if (this->index != other.index)
         return false;
      return true;
   }

   bool wrapper::should_expose_to_script() const noexcept {
      if (this->type == wrapper_type::form_data) {
         if (!this->stub)
            return false;
         auto* stub = this->stub;
         if (stub->formType == dovah::form_type::setting)
            return false;
         if (stub->is_none_stub())
            return false;
      }
      return true;
   }

   bool wrapper::is_descendant_of(const wrapper& other) const noexcept {
      if (this->depth < other.depth)
         return false;
      if (this->depth == other.depth) {
         if (!other.is_collection)
            return false;
         if (this->is_collection)
            return false;
      }
      if (this->type == wrapper_type::form_data) {
         if (this->stub != other.stub)
            return false;
      }
      for (uint8_t i = 0; i < other.depth; ++i)
         if (this->parts[i] != other.parts[i])
            return false;
      return true;
   }
   bool wrapper::is_equal(const wrapper* other) const noexcept {
      if (this->type != other->type)
         return false;
      if (this->type == wrapper_type::form_data) {
         if (this->stub != other->stub)
            return false;
      } else if (this->type == wrapper_type::ui) {
         if (this->widget != other->widget)
            return false;
         if (this->model_observer != other->model_observer)
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
   bool wrapper::is_in_same_collection(const wrapper& other) const noexcept {
      if (this->type != other.type)
         return false;
      if (this->type == wrapper_type::form_data) {
         if (this->stub != other.stub)
            return false;
      }
      if (this->depth != other.depth)
         return false;
      if (this->is_collection | other.is_collection)
         return false;
      uint8_t i = 0;
      for (; i < (signed int)(this->depth) - 1; ++i)
         if (this->parts[i] != other.parts[i])
            return false;
      if (this->parts[i].signature != other.parts[i].signature)
         return false;
      return true;
   }

   void wrapper::load_form() {
      if (!this->stub)
         return;
      this->form = this->stub->load();
   }
   void wrapper::before_edit() {
      if (!this->stub)
         return;
      emit DovahKitCore::get().formModificationImminent(this->stub);
   }
   void wrapper::after_edit() {
      if (!this->stub)
         return;
      this->stub->set_edited(true);
      emit DovahKitCore::get().formModified(this->stub);
   }

   void wrapper::error_if_wrong_form_type(lua_State* L, int arg_index, dovah::form_type_t ft, bool loose) {
      luaL_argcheck(L, this->type == wrapper_type::form_data, arg_index, "form expected");
      luaL_argcheck(L, this->depth == 0, arg_index, "form expected");
      if (this->stub)
         luaL_argcheck(L, this->stub->formType == ft, arg_index, "incorrect form type");
   }

   int8_t wrapper::depth_of(const cobb::eight_cc& code) const noexcept {
      for (int8_t i = 0; i < part_count; ++i)
         if (this->parts[i].signature == code)
            return i;
      return -1;
   }
   bool wrapper::is_collection_at_depth(uint8_t d) const noexcept {
      if (this->depth != d + 1)
         return false;
      return this->is_collection;
   }
}