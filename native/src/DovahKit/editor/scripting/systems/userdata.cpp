#include "userdata.h"
#include "editor_script_inner_core.h"
#include "lua_managed_resources.h"

#include "../class_killer.h"
#include "../../../helpers/lua/discontiguous_list.h"
#include "../../../helpers/lua/dump.h"
#include "../../../helpers/lua/isempty.h"

#include "../../form_stub_meta_type.h" // needed for QVariants of form stub pointers
#include "../../../dovah/forms/Form.h" // needed for working with any loaded_form_ptr
#include "../widgets/objects/LuaScriptableCanvasWidgetLayerData.h"

//
// Given a dovah::form_stub& named stub:
// 
//    __lua_registry[wrapper_storage_registry_key][&stub] == { wrapper, wrapper, wrapper }
//
// Lua allows us to use void pointers as "light userdata," essentially allowing us to use 
// raw pointers as keys or values in Lua tables.
//

void DovahKitScriptVMUserdataInterface::remove(editor_script::wrapper& instance) {
   DovahKitScriptVMCore::require_script_thread();
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   auto  index = instance.last_part().index;
   void* light = instance.get_pertinent_pointer();
   //
   std::vector<int> refs_to_sever;
   if (instance.lua_key != LUA_NOREF)
      //
      // The instance can lack a Lua key if we're trying to zombify a wrapper "remotely," i.e. 
      // rather than receiving a wrapper from Lua and zombifying it, we want to zombify a 
      // wrapper that may or may not exist in Lua by constructing a wrapper on the stack and 
      // then passing it into here.
      //
      refs_to_sever.push_back(instance.lua_key);
   //
   auto si_storage = start + 1;
   auto si_nk      = start + 2;
   auto si_nv      = start + 3;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_storage_registry_key); // push 1
   lua_pushlightuserdata(L, light);
   lua_rawget(L, si_storage); // STACK: - [ ..., storage_root, storage_root[light] ] +
   assert(lua_istable(L, -1));
   lua_copy  (L, -1, si_storage);
   lua_settop(L, si_storage); // STACK: - [ ..., storage_root[light] ] +
   //
   // If the wrapper to be removed is in a sequential collection, fix up the indices of all 
   // of its next-siblings. Either way, identify and track the keys of any child/descendant 
   // wrappers.
   //
   lua_pushnil(L); // nk
   while (lua_next(L, si_storage) != 0) {
      editor_script::wrapper* other = nullptr;
      if (lua_type(L, si_nv) == LUA_TUSERDATA)
         other = (editor_script::wrapper*) lua_touserdata(L, si_nv);
      //
      lua_settop(L, si_nk);
      //
      assert(other->lua_key != LUA_NOREF);
      if (!other || other->lua_key == instance.lua_key)
         continue;
      if (instance.is_in_same_collection(*other)) {
         if (!instance.last_part().noncontiguous) {
            auto& o_last = other->last_part();
            if (index < o_last.index)
               //
               // reduce (other), as a previous sibling has been deleted.
               //
               --o_last.index;
         }
      } else if (other->is_descendant_of(instance)) {
         refs_to_sever.push_back(other->lua_key);
      } else if (instance.lua_key == LUA_NOREF) {
         if (instance.is_equal(other))
            refs_to_sever.push_back(other->lua_key);
      }
   }
   //
   // Zombify and forget the wrapper and all of its descendants.
   //
   lua_settop(L, si_storage);
   for (auto key : refs_to_sever) {
      lua_pushcfunction(L, &editor_script::zombify_userdata); // prepare to make a Lua call...
      lua_rawgeti      (L, si_storage, key);
      //
      auto* target = (editor_script::wrapper*) lua_touserdata(L, -1);
      assert(target && target->lua_key == key);
      target->lua_key = LUA_NOREF;
      target->stub    = nullptr; // need to sever this now, because we won't be able to if, say, the form is deleted after we forget about this wrapper
      target->form    = nullptr;
      //
      lua_call(L, 1, 0); // ...and then, after we've adjusted the native wrapper, make the call.
      //
      cobb::lua::discontiguous_list::remove(L, si_storage, key); // remove the target from storage.
   }
   //
   lua_pushnil(L);
   if (lua_next(L, si_storage) == 0) { // table is empty
      //
      // If the list of wrappers for this pointer is empty, delete the list itself.
      //
      lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_storage_registry_key);
      lua_pushlightuserdata(L, light);
      lua_pushnil(L);
      lua_rawset(L, -3);
   }
   //
   lua_settop(L, start);
}

void DovahKitScriptVMUserdataInterface::remove_form(dovah::form_stub& stub) {
   DovahKitScriptVMCore::require_script_thread();
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   //
   auto si_storage = start + 1;
   auto si_nk      = start + 2;
   auto si_nv      = start + 3;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_storage_registry_key); // push 1
   lua_pushlightuserdata(L, &stub);
   lua_rawget(L, si_storage); // STACK: - [ ..., storage_root, storage_root[light] ] +
   if (!lua_istable(L, -1)) {
      lua_settop(L, start);
      return;
   }
   lua_copy  (L, -1, si_storage);
   lua_settop(L, si_storage); // STACK: - [ ..., storage_root[light] ] +
   //
   // Zombify all wrappers for this form and its parts.
   //
   lua_pushnil(L); // nk
   while (lua_next(L, si_storage) != 0) {
      editor_script::wrapper* other = nullptr;
      if (lua_type(L, si_nv) == LUA_TUSERDATA) {
         if (auto* target = (editor_script::wrapper*) lua_touserdata(L, si_nv)) {
            assert(target->stub == &stub);
            target->lua_key = LUA_NOREF;
            target->stub    = nullptr; // need to sever this now, because we won't be able to if, say, the form is deleted after we forget about this wrapper
            target->form    = nullptr;
            //
            lua_pushcfunction(L, &editor_script::zombify_userdata);
            lua_pushvalue    (L, si_nv);
            lua_call(L, 1, 0);
         }
      }
      //
      lua_settop(L, si_nk);
   }
   //
   // Erase the table for this form.
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_storage_registry_key);
   lua_pushlightuserdata(L, &stub);
   lua_pushnil(L);
   lua_rawset(L, -3);
   //
   lua_settop(L, start);
}

void DovahKitScriptVMUserdataInterface::remove_model_observer(ObservableStandardItemModelObserver& observer) {
   DovahKitScriptVMCore::require_script_thread();
   auto* L     = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   //
   auto si_storage = start + 1;
   auto si_nk      = start + 2;
   auto si_nv      = start + 3;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_storage_registry_key); // push 1
   lua_pushlightuserdata(L, &observer);
   lua_rawget(L, si_storage); // STACK: - [ ..., storage_root, storage_root[light] ] +
   if (!lua_istable(L, -1)) {
      lua_settop(L, start);
      return;
   }
   lua_copy  (L, -1, si_storage);
   lua_settop(L, si_storage); // STACK: - [ ..., storage_root[light] ] +
   //
   // Zombify all wrappers for this form and its parts.
   //
   lua_pushnil(L); // nk
   while (lua_next(L, si_storage) != 0) {
      editor_script::wrapper* other = nullptr;
      if (lua_type(L, si_nv) == LUA_TUSERDATA) {
         if (auto* target = (editor_script::wrapper*) lua_touserdata(L, si_nv)) {
            assert(target->model_observer == &observer);
            target->lua_key = LUA_NOREF;
            // Don't clear the model-observer pointer from the wrapper here; let the __gc metamethod's call to wrapper::teardown handle that (and the observer's refcount) instead.
            //
            lua_pushcfunction(L, &editor_script::zombify_userdata);
            lua_pushvalue    (L, si_nv);
            lua_call(L, 1, 0);
         }
      }
      //
      lua_settop(L, si_nk);
   }
   //
   // Erase the table for this form.
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_storage_registry_key);
   lua_pushlightuserdata(L, &observer);
   lua_pushnil(L);
   lua_rawset(L, -3);
   //
   lua_settop(L, start);
}

bool DovahKitScriptVMUserdataInterface::wrapper_exists_for(void* pertinent_pointer) {
   DovahKitScriptVMCore::require_script_thread();
   auto* L      = this->vm.lua_vm;
   auto  start  = lua_gettop(L);
   bool  result = false;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_storage_registry_key);
   lua_pushlightuserdata(L, pertinent_pointer);
   lua_rawget(L, -2);
   if (lua_istable(L, -1)) {
      result = !cobb::lua::isempty(L, -1);
   }
   lua_settop(L, start);
   return result;
}

void DovahKitScriptVMUserdataInterface::prune_wrapper_list_for(editor_script::wrapper& instance) {
   if (this->vm.teardown_in_progress()) {
      //
      // The Lua state is not externally accessible during teardown, but we also don't 
      // need to do any sort of intelligent object lifetime management during teardown 
      // because we're just gonna delete everything anyway.
      //
      return;
   }
   //
   void* light = instance.get_pertinent_pointer();
   if (!light)
      return;
   auto* L = this->vm.lua_vm;
   assert(L);
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_storage_registry_key);
   lua_pushlightuserdata(L, light);
   lua_rawget(L, -2);
   bool destroy = !lua_istable(L, -1);
   if (!destroy)
      destroy = cobb::lua::isempty(L, -1);
   lua_pop(L, 1);
   //
   if (destroy) {
      //
      // The wrapper list is empty. This means a few things:
      // 
      //  - The wrapped object is no longer referred to within Lua.
      // 
      //  - We may end up destroying the wrapped object, in which case we'll be 
      //    left with an empty list keyed to a dangling pointer.
      // 
      // We should destroy the list just to keep things clean. We can also use this 
      // as an opportunity to manage object lifetimes. It is theoretically possible 
      // for multiple Lua userdata to refer to [different parts of] the same under-
      // lying object (the wrapper's "pertinent pointer"), and if we need to know 
      // when the object is referenced by Lua (in order to manage its lifetime), 
      // then we need to either:
      // 
      //  - Track a refcount for it within the VM core, and update that refcount 
      //    every time any userdata for the object is created or destroyed.
      // 
      // or:
      // 
      //  - Don't bother tracking a refcount; just act on the object here, in this 
      //    very function below this very code comment, since we know at this point 
      //    that no userdata point to the object anymore.
      // 
      // We're going with the latter approach.
      //
      lua_pushlightuserdata(L, light);
      lua_pushnil(L);
      lua_rawset(L, -3);
      //
      switch (instance.type) {
         using wt = editor_script::wrapper_type;
         case wt::lua_managed_resource:
            {
               auto* mr = instance.managed_resource;
               mr->is_lua_referenced = false;
               DovahKitScriptVMResourceInterface::get().on_resource_unreferenced(*mr);
            }
            break;
         case wt::ui:
            this->vm.widget_no_longer_referenced(instance.widget);
            break;
         case wt::ui_button_group:
            this->vm.button_group_no_longer_referenced(instance.button_group);
            break;
         case wt::ui_model_item:
            this->vm.model_observer_no_longer_referenced(instance.model_observer);
            break;
         case wt::ui_canvas_layer:
            // TODO?
            break;
         case wt::ui_canvas_layer_data:
            this->vm.canvas_layer_data_unreferenced(instance.canvas_layer_data);
            break;
      }
   }
}

int DovahKitScriptVMUserdataInterface::push(lua_State* L, const editor_script::wrapper& instance, const char* metatable_name) {
   DovahKitScriptVMCore::require_script_thread();
   if (!instance.should_expose_to_script())
      return 0;
   //
   auto  start = lua_gettop(L);
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_storage_registry_key); // push 1
   auto  table = lua_gettop(L);
   //
   auto si_storage = start + 1;
   auto si_nk      = start + 2;
   auto si_nv      = start + 3;
   auto si_created = si_storage + 1;
   //
   void* light = instance.get_pertinent_pointer();
   assert(light != nullptr && "Why does a wrapper have a nullptr pertinent pointer? These need to be unique among top-level wrapped objects (e.g. entire forms as opposed to form parts) so that they can be used as keys. Configure the wrapper properly before pushing it!");
   {  // Get the list of wrappers for this pointer
      lua_pushlightuserdata(L, light); // push 1
      lua_rawget(L, table);            // push 0 // STACK: - [ ..., storage_root, storage_root[light] ] +
      if (!lua_istable(L, -1)) {
         lua_settop(L, table);
         //
         lua_createtable (L, 0, 0); // push 1 // storage_sub = {}
         lua_getfield    (L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_weakmap_metatable_key);
         lua_setmetatable(L, -2);
         lua_pushlightuserdata(L, light);     // push 1
         lua_pushvalue        (L, table + 1); // push 1
         lua_rawset           (L, table);     // pop  2 // STACK: - [ ..., storage_root, storage_root[light] ] + // storage_root[wrapper_pointer] = storage_sub
      }
      lua_copy  (L, -1, table);
      lua_settop(L, table); // STACK: - [ ..., storage_root[light] ] + // table = registry[wrapper_storage_registry_key][wrapper_pointer] or {}
   }
   //
   // Check for an existing identical wrapper:
   //
   lua_pushnil(L); // push 1
   while (lua_next(L, table) != 0) { // push 2 (only if truthy)
      auto* existing = (editor_script::wrapper*) lua_touserdata(L, si_nv);
      #if _DEBUG
         if (!existing) {
            cobb::lua::print_stack_and_vars(L);
            __debugbreak();
         }
      #endif
      if (existing->is_equal(&instance)) {
         lua_copy  (L, si_nv, table); // move the value
         lua_settop(L, table);
         return 1;
      }
      lua_pop(L, 1); // pop the value; keep the key for the next iteration
   }
   //
   assert(lua_gettop(L) == si_storage);
   //
   // Create a new wrapper.
   //
   auto* ptr = (editor_script::wrapper*) lua_newuserdatauv(L, sizeof(editor_script::wrapper), 0); // push 1
   new (ptr) editor_script::wrapper;
   *ptr = instance;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, metatable_name); // push 1
   if (lua_isnoneornil(L, -1)) {
      assert(false && "The wrapper-class wasn't set up properly; its metatable is undefined.");
      lua_settop(L, start);
      return 0;
   }
   lua_setmetatable(L, si_created); // pop 1
   //
   lua_pushvalue(L, si_created); // push 1 // push another reference to the wrapper onto the stack, as the next function will remove whichever reference it uses
   ptr->lua_key = cobb::lua::discontiguous_list::insert(L, si_storage);
   ptr->_on_pushed();
   //
   lua_remove(L, -2);
   return 1;
}

void DovahKitScriptVMUserdataInterface::remove_from_sequential_collection(editor_script::wrapper& to_remove) {
   DovahKitScriptVMCore::require_script_thread();
   assert(to_remove.depth && !to_remove.is_collection && "The (to_remove) argument must be an element in a sequential collection.");
   this->remove(to_remove);
}

void DovahKitScriptVMUserdataInterface::clear_entire_collection(editor_script::wrapper& w) {
   DovahKitScriptVMCore::require_script_thread();
   assert(w.depth && w.is_collection && "This function must be given a collection wrapper.");
   auto* L = this->vm.lua_vm;
   auto  start = lua_gettop(L);
   auto  index = w.last_part().index;
   void* light = w.get_pertinent_pointer();
   //
   std::vector<int> refs_to_sever;
   //
   auto si_storage = start + 1;
   auto si_nk      = start + 2;
   auto si_nv      = start + 3;
   //
   lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::wrapper_storage_registry_key); // push 1
   lua_pushlightuserdata(L, light);
   lua_rawget(L, si_storage); // STACK: - [ ..., storage_root, storage_root[light] ] +
   assert(lua_istable(L, -1));
   lua_copy  (L, -1, si_storage);
   lua_settop(L, si_storage); // STACK: - [ ..., storage_root[light] ] +
   //
   // Identify and track the keys of any child/descendant wrappers.
   //
   lua_pushnil(L); // nk
   while (lua_next(L, si_storage) != 0) {
      editor_script::wrapper* other = nullptr;
      if (lua_type(L, si_nv) == LUA_TUSERDATA)
         other = (editor_script::wrapper*) lua_touserdata(L, si_nv);
      //
      lua_settop(L, si_nk);
      //
      if (!other || other->lua_key == w.lua_key)
         continue;
      if (other->is_descendant_of(w)) {
         refs_to_sever.push_back(other->lua_key);
      }
   }
   //
   // Zombify and forget the wrapper's descendants.
   //
   lua_settop(L, si_storage);
   for (auto key : refs_to_sever) {
      lua_pushcfunction(L, &editor_script::zombify_userdata); // prepare to make a Lua call...
      lua_rawgeti      (L, si_storage, key);
      //
      auto* target = (editor_script::wrapper*) lua_touserdata(L, -1);
      assert(target && target->lua_key == key);
      target->lua_key = LUA_NOREF;
      target->stub    = nullptr; // need to sever this now, because we won't be able to if, say, the form is deleted after we forget about this wrapper
      target->form    = nullptr;
      //
      lua_call(L, 1, 0); // ...and then, after we've adjusted the native wrapper, make the call.
      //
      cobb::lua::discontiguous_list::remove(L, si_storage, key); // remove the target from storage.
   }
   //
   lua_settop(L, start);

}