#include "userdata.h"
#include "coordinator.h"
#include "lifetime.h"
#include "resources.h"
#include "../class_zombification.h"
#include "../verify_threading.h"
#include "../../wrapper.h"

#include "../../../helpers/lua/dump.h"
#include "../../../helpers/lua/isempty.h"

#include "../../../dovah/forms/Form.h" // needed for working with any loaded_form_ptr
#include "../../../editor/form_stub_meta_type.h" // needed for QVariants of form stub pointers
#include "../../qt/LuaScriptableCanvasWidgetLayerData.h"

//
// Given a dovah::form_stub& named stub:
// 
//    __lua_registry[wrapper_storage_registry_key][&stub] == { wrapper, wrapper, wrapper }
//
// Lua allows us to use void pointers as "light userdata," essentially allowing us to use 
// raw pointers as keys or values in Lua tables.
//

namespace {
   static constexpr const char* wrapper_storage_registry_key  = "dovahscript.internal.wrapper_storage";
   static constexpr const char* wrapper_weakmap_metatable_key = "dovahscript.internal.wrapper_storage_metatable";

   inline lua_State* _get_lua() {
      return dovahscript::core::subsystems::coordinator::get().lua_state;
   }
}

namespace dovahscript::core::subsystems {
   void userdata::initialize(lua_State* L) {
      lua_createtable(L, 0, 0);
      lua_setfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key);
      //
      lua_createtable(L, 0, 1);
      lua_pushstring(L, "v");
      lua_setfield  (L, -2, "__mode");
      lua_setfield(L, LUA_REGISTRYINDEX, wrapper_weakmap_metatable_key);
   }

   void userdata::destroy(wrapper& instance) {
      require_script_thread();
      //
      auto* L     = _get_lua();
      auto  start = lua_gettop(L);
      auto  index = instance.last_part().index;
      void* light = instance.pertinent_pointer;
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
      lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key); // push 1
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
         wrapper* other = nullptr;
         if (lua_type(L, si_nv) == LUA_TUSERDATA)
            other = (wrapper*) lua_touserdata(L, si_nv);
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
         lua_pushcfunction(L, &dovahscript::zombify_userdata); // prepare to make a Lua call...
         lua_rawgeti      (L, si_storage, key);
         //
         auto* target = (wrapper*) lua_touserdata(L, -1);
         assert(target && target->lua_key == key);
         target->lua_key = LUA_NOREF;
         target->stub    = nullptr; // need to sever this now, because we won't be able to if, say, the form is deleted after we forget about this wrapper
         target->form    = nullptr;
         //
         lua_call(L, 1, 0); // ...and then, after we've adjusted the native wrapper, make the call.
         //
         if (key > 0) { // remove the target from storage.
            lua_pushnil(L);
            lua_rawseti(L, si_storage, key);
         }
      }
      //
      lua_pushnil(L);
      if (lua_next(L, si_storage) == 0) { // table is empty
         //
         // If the list of wrappers for this pointer is empty, delete the list itself.
         //
         lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key);
         lua_pushlightuserdata(L, light);
         lua_pushnil(L);
         lua_rawset(L, -3);
      }
      //
      lua_settop(L, start);
   }

   void userdata::destroy_all(dovah::form_stub& stub) {
      require_script_thread();
      //
      auto* L     = _get_lua();
      auto  start = lua_gettop(L);
      //
      auto si_storage = start + 1;
      auto si_nk      = start + 2;
      auto si_nv      = start + 3;
      //
      lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key); // push 1
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
         wrapper* other = nullptr;
         if (lua_type(L, si_nv) == LUA_TUSERDATA) {
            if (auto* target = (wrapper*) lua_touserdata(L, si_nv)) {
               assert(target->stub == &stub);
               target->lua_key = LUA_NOREF;
               target->stub    = nullptr; // need to sever this now, because we won't be able to if, say, the form is deleted after we forget about this wrapper
               target->form    = nullptr;
               //
               lua_pushcfunction(L, &dovahscript::zombify_userdata);
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
      lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key);
      lua_pushlightuserdata(L, &stub);
      lua_pushnil(L);
      lua_rawset(L, -3);
      //
      lua_settop(L, start);
   }

   void userdata::destroy_all(ObservableStandardItemModelObserver& observer) {
      require_script_thread();
      //
      auto* L     = _get_lua();
      auto  start = lua_gettop(L);
      //
      auto si_storage = start + 1;
      auto si_nk      = start + 2;
      auto si_nv      = start + 3;
      //
      lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key); // push 1
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
         wrapper* other = nullptr;
         if (lua_type(L, si_nv) == LUA_TUSERDATA) {
            if (auto* target = (wrapper*) lua_touserdata(L, si_nv)) {
               assert(target->model_observer == &observer);
               target->lua_key = LUA_NOREF;
               // Don't clear the model-observer pointer from the wrapper here; let the __gc metamethod's call to wrapper::teardown handle that (and the observer's refcount) instead.
               //
               lua_pushcfunction(L, &dovahscript::zombify_userdata);
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
      lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key);
      lua_pushlightuserdata(L, &observer);
      lua_pushnil(L);
      lua_rawset(L, -3);
      //
      lua_settop(L, start);
   }

   bool userdata::wrapper_exists_for(void* pertinent_pointer) {
      require_script_thread();
      //
      auto* L      = _get_lua();
      auto  start  = lua_gettop(L);
      bool  result = false;
      //
      lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key);
      lua_pushlightuserdata(L, pertinent_pointer);
      lua_rawget(L, -2);
      if (lua_istable(L, -1)) {
         result = !cobb::lua::isempty(L, -1);
      }
      lua_settop(L, start);
      return result;
   }

   void userdata::on_wrapper_destroyed(wrapper& instance) {
      require_script_thread();
      //
      auto& coordinator_s = coordinator::get();
      //
      if (coordinator_s.teardown_in_progress()) {
         //
         // We don't need to do any sort of intelligent object lifetime management during 
         // teardown, because we're just gonna delete everything anyway.
         //
         return;
      }
      //
      void* light = instance.pertinent_pointer;
      if (!light)
         return;
      auto* L = coordinator_s.lua_state;
      assert(L);
      //
      lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key);
      lua_pushlightuserdata(L, light);
      lua_rawget(L, -2);
      if (!lua_istable(L, -1)) {
         lua_pop(L, 2);
         //
         // Here's an interesting edge-case that can happen: what if two wrappers for the 
         // same pertinent pointer become unused at around the same time, and there are no 
         // other wrappers for that pertinent pointer? Well, both of those wrappers would 
         // run their __gc metamethods one after the other, and both of those metamethods 
         // would eventually lead to this function here.
         // 
         // The first time we get here, we'd find an empty wrapper list for that pertinent 
         // pointer, and so we'd run the "Lua-unreferenced" behavior for the pertinent 
         // pointer. Commonly, this will involve queuing the deletion of the pointed-to 
         // object. We'd also delete the empty list from Lua. That's the first dead wrapper 
         // done.
         // 
         // The second dead wrapper, however, would still run its own __gc function, which 
         // would still lead here... and this time, it'd find that there's no wrapper list 
         // at all. If, at this point, we run the "Lua-unreferenced" behavior, then we'll 
         // have run it twice, which can lead to double-frees on wrapped objects. Instead, 
         // if we see that there's no list, then we need to trust that the pointed-to 
         // object was already dealt with by some other wrapper that was deleted at around 
         // the same time as us, and we need to simply exit.
         // 
         // (But how is the list empty? Remember: __gc is used for final deletion, but the 
         // wrappers will have disappeared from the list, a weakmap, earlier, if they 
         // became unused by the script at around the same time.)
         //
         return;
      }
      bool destroy = cobb::lua::isempty(L, -1);
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
      }
      lua_pop(L, 1); // remove wrapper storage table from the stack
      //
      if (destroy) {
         auto& lifetime_s = lifetime::get();
         switch (instance.type) {
            using wt = wrapper_type;
            case wt::lua_managed_resource:
               {
                  auto* mr = instance.managed_resource;
                  mr->is_lua_referenced = false;
                  resources::get().on_resource_unreferenced(*mr);
               }
               break;
            case wt::widget:
               lifetime_s.on_lua_unreferenced(cobb::passkey<lifetime,userdata>(), instance.widget);
               break;
            case wt::button_group:
               lifetime_s.on_lua_unreferenced(cobb::passkey<lifetime,userdata>(), instance.button_group);
               break;
            case wt::model_observer:
               lifetime_s.on_lua_unreferenced(cobb::passkey<lifetime,userdata>(), instance.model_observer);
               break;
            case wt::canvas_entity:
               // These objects are managed by the CanvasWidget to which they belong. Don't 
               // mess with them here.
               break;
            case wt::canvas_layer_data:
               lifetime_s.on_lua_unreferenced(cobb::passkey<lifetime,userdata>(), instance.canvas_layer_data);
               break;
         }
      }
   }

   int userdata::push(lua_State* L, const wrapper& instance, const char* metatable_name) {
      require_script_thread();
      if (!instance.should_expose_to_script())
         return 0;
      //
      auto  start = lua_gettop(L);
      lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key); // push 1
      auto  table = lua_gettop(L);
      //
      auto si_storage = start + 1;
      auto si_nk      = start + 2;
      auto si_nv      = start + 3;
      auto si_created = si_storage + 1;
      //
      void* light = instance.pertinent_pointer;
      assert(light != nullptr && "Why does a wrapper have a nullptr pertinent pointer? These need to be unique among top-level wrapped objects (e.g. entire forms as opposed to form parts) so that they can be used as keys. Configure the wrapper properly before pushing it!");
      {  // Get the list of wrappers for this pointer
         lua_pushlightuserdata(L, light); // push 1
         lua_rawget(L, table);            // push 0 // STACK: - [ ..., storage_root, storage_root[light] ] +
         if (!lua_istable(L, -1)) {
            lua_settop(L, table);
            //
            lua_createtable (L, 0, 0); // push 1 // storage_sub = {}
            lua_getfield    (L, LUA_REGISTRYINDEX, wrapper_weakmap_metatable_key);
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
         auto* existing = (wrapper*) lua_touserdata(L, si_nv);
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
      auto* ptr = (wrapper*) lua_newuserdatauv(L, sizeof(wrapper), 0); // push 1
      new (ptr) wrapper;
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
      {
         int i = lua_rawlen(L, si_storage) + 1;
         lua_rawseti(L, si_storage, i);
         ptr->lua_key = i;
      }
      ptr->_on_pushed();
      //
      lua_remove(L, -2);
      return 1;
   }

   void userdata::remove_from_sequential_collection(wrapper& to_remove) {
      require_script_thread();
      assert(to_remove.depth && !to_remove.is_collection && "The (to_remove) argument must be an element in a sequential collection.");
      this->destroy(to_remove);
   }

   void userdata::clear_entire_collection(wrapper& w) {
      require_script_thread();
      assert(w.depth && w.is_collection && "This function must be given a collection wrapper.");
      auto* L     = _get_lua();
      auto  start = lua_gettop(L);
      auto  index = w.last_part().index;
      void* light = w.pertinent_pointer;
      //
      std::vector<int> refs_to_sever;
      //
      auto si_storage = start + 1;
      auto si_nk      = start + 2;
      auto si_nv      = start + 3;
      //
      lua_getfield(L, LUA_REGISTRYINDEX, wrapper_storage_registry_key); // push 1
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
         wrapper* other = nullptr;
         if (lua_type(L, si_nv) == LUA_TUSERDATA)
            other = (wrapper*) lua_touserdata(L, si_nv);
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
         lua_pushcfunction(L, &dovahscript::zombify_userdata); // prepare to make a Lua call...
         lua_rawgeti      (L, si_storage, key);
         //
         auto* target = (wrapper*) lua_touserdata(L, -1);
         assert(target && target->lua_key == key);
         target->lua_key = LUA_NOREF;
         target->stub    = nullptr; // need to sever this now, because we won't be able to if, say, the form is deleted after we forget about this wrapper
         target->form    = nullptr;
         //
         lua_call(L, 1, 0); // ...and then, after we've adjusted the native wrapper, make the call.
         //
         if (key > 0) { // remove the target from storage.
            lua_pushnil(L);
            lua_rawseti(L, si_storage, key);
         }
      }
      //
      lua_settop(L, start);

   }
}