#include "wrapper.h"
#include <cassert>
#include "core/subsystems/lifetime.h"
#include "core/subsystems/resources.h"
#include "core/subsystems/userdata.h"
#include "qt/DovahscriptCanvasWidgetLayerData.h"
#include "../ui/generic/ObservableStandardItemModel.h"

// For signaling before and after a form is edited:
#include "core/subsystems/coordinator.h"
#include "tasks/s2m/lambda.h"
#include "../editor/core.h"

namespace dovahscript {
   /*static*/ int wrapper::__close(lua_State* L) {
      //
      // A wrapper will run this metamethod if it is stored in a local variable with the <close> 
      // attribute and if that variable goes out of scope, e.g.
      // 
      //    do
      //       local r <close> = raster.new(...)
      //       my_dropdown.items[2].icon = r
      //    end
      // 
      // Lua scripts, then, can decide to have a wrapper torn down as soon as possible. This is 
      // encouraged when working with Lua-managed resources, as it will ensure that they are not 
      // kept alive for longer than they need to be -- helpful if we decide to cap the total 
      // memory usage of these resources. (Of course, it may be friendlier to users to just have 
      // Lua run two GC passes before creating a resource, if the resource subsystem says that 
      // it has no room for the desired resource.)
      //
      auto* userdata = (wrapper*)lua_touserdata(L, 1);
      userdata->teardown(true);
      return 0;
   }
   /*static*/ int wrapper::__gc(lua_State* L) {
      auto* userdata = (wrapper*)lua_touserdata(L, 1);
      userdata->teardown();
      userdata->~wrapper();
      lua_pushnil(L);
      lua_setmetatable(L, 1); // Lua can't guarantee that __gc will only be called once, so make sure there *is* no __gc to call a second time
      return 0;
   }
}

namespace dovahscript {
   wrapper::~wrapper() {
      //
      // Don't run teardown code in the destructor. The way wrappers work is that a Lua API 
      // creates a given wrapper and asks our userdata interface to push that wrapper onto 
      // the Lua stack, which returns it to the script. However, the userdata interface will 
      // first check to see if there's already an identical wrapper active in the script, 
      // and if so, it returns that instead. This means that if a Lua API is called twice 
      // and returns wrappers to the same object, those wrappers compare equal (without us 
      // needing to override __eq or other nonsense).
      // 
      // A consequence of it working this way is that the wrapper that a Lua API creates is 
      // never actually used directly; it's *copied* into Lua, and the original then goes 
      // out of scope when the Lua API ends, and is destroyed. So, if we run teardown code 
      // when a wrapper is destroyed, these not-actually-in-the-script wrappers will be 
      // torn down, which will... confuse our script core.
      //
   }

   void wrapper::teardown(bool is_toclose) {
      if (this->type == wrapper_type::undefined) {
         //
         // This can happen if the wrapper was closed.
         //
         return;
      }
      //
      core::subsystems::userdata::get().on_wrapper_destroyed(*this, is_toclose);
      this->pertinent_pointer = nullptr;
      //
      this->type = wrapper_type::undefined;
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

   void wrapper::_on_pushed() {
      switch (this->type) {
         using lifetime_t = core::subsystems::lifetime;
         case wrapper_type::model_observer:
            lifetime_t::get().on_lua_referenced(cobb::passkey<lifetime_t, wrapper>(), this->model_observer);
            return;
         case wrapper_type::lua_managed_resource:
            assert(this->managed_resource);
            this->managed_resource->is_lua_referenced = true;
            return;
         case wrapper_type::canvas_layer_data:
            assert(this->canvas_layer_data);
            this->canvas_layer_data->is_lua_referenced = true;
            return;
      }
   }

   bool wrapper::part::operator==(const part& other) const noexcept {
      if (this->signature != other.signature)
         return false;
      if (this->index != other.index)
         return false;
      return true;
   }

   bool wrapper::should_expose_to_script() const noexcept {
      if (this->type == wrapper_type::form) {
         if (!this->stub)
            return false;
         auto* stub = this->stub;
         if (stub->formType == dovah::form_type::setting)
            return false;
         if (stub->is_none_stub())
            return false;
      } else if (this->type == wrapper_type::model_observer) {
         if (!this->model_observer)
            return false;
         if (!this->model_observer->isValid())
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
      if (this->type == wrapper_type::form) {
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
      if (this->pertinent_pointer != other->pertinent_pointer)
         return false;
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
      if (this->pertinent_pointer != other.pertinent_pointer)
         return false;
      if (this->depth != other.depth)
         return false;
      if (this->is_collection || other.is_collection)
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
      //
      // We need to emit the form-modification-imminent signal, but to avoid race conditions 
      // within the rest of the editor frontend and possibly even within the backend, we need 
      // to ensure that we perform this operation in lockstep: script execution cannot be 
      // allowed to continue until the signal is emitted and responded to.
      //
      // If we emit the signal on our own thread, then it will trigger a queued connection, 
      // which means that the script thread may be able to modify the form before the signal 
      // is responded to. There are a few systems that will break if this occurs; for example, 
      // the Object Window will not be able to accurately maintain Use Info counts when one 
      // form is modified to no longer use another, because in order to detect that case, it 
      // has to pre-cache the former's outbound connections when form modification is imminent 
      // (but, explicitly, before it has occurred) and then compare that to the outbound 
      // connections that remain when the form modification is complete.
      //
      // If we emit the signal on the main thread, then it will trigger a direct connection, 
      // calling any registered slots and handlers immediately and synchronously. If we wait 
      // on this (e.g. by using our messaging system to effect it), then we, too, will block.
      //
      // Firing messages from within the wrapper internals feels like a disgusting hack and 
      // a total failure of encapsulation. And it is! But if it works, it works.
      //
      auto* task    = new dovahscript::tasks::s2m::lambda(true);
      auto* stub    = this->stub;
      task->handler = [stub]() {
         emit DovahKitCore::get().formModificationImminent(stub);
      };
      core::subsystems::coordinator::get().send_script_task(*task);
      delete task;
   }
   void wrapper::after_edit() {
      if (!this->stub)
         return;
      this->stub->set_edited(true);
      //
      // As with the form-modification-imminent signal, we should emit the form-modified 
      // signal in lockstep for safety's sake.
      //
      auto* task    = new dovahscript::tasks::s2m::lambda(true);
      auto* stub    = this->stub;
      task->handler = [stub]() {
         emit DovahKitCore::get().formModified(stub);
      };
      core::subsystems::coordinator::get().send_script_task(*task);
      delete task;
   }

   void wrapper::error_if_wrong_form_type(lua_State* L, int arg_index, dovah::form_type_t ft, bool loose) {
      luaL_argcheck(L, this->type == wrapper_type::form, arg_index, "form expected");
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