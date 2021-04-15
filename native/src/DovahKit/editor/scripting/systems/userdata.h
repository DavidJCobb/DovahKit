#pragma once
#include "../../../helpers/singleton.h"
#include "../wrapper.h"
#include "editor_script_inner_core.h"

class DovahKitScriptVMUserdataInterface : cobb::singleton {
   //
   // This is an interface to DovahKitScriptVM, provided for the benefit of our userdata internals.
   //
   protected:
      DovahKitScriptVMUserdataInterface(DovahKitScriptVMCore& w) : vm(w) {}
   public:
      static DovahKitScriptVMUserdataInterface& get() {
         static DovahKitScriptVMUserdataInterface instance(DovahKitScriptVMCore::get());
         return instance;
      }
      //
      DovahKitScriptVMCore& vm;

      //
      // Remove a wrapper's metatable, and then remove it from the wrapper storage table. Effectively 
      // "kills" the wrapper. The wrapper will be deleted later, when Lua garbage-collects it.
      //
      void remove(editor_script::wrapper&);

      //
      // Kills all wrappers for the given form and any of its parts.
      //
      void remove_form(dovah::form_stub&);

      void remove_model_observer(ObservableStandardItemModelObserver&);

      bool wrapper_exists_for(void*);

      //
      // Check if Lua already has an identical copy of the passed-in wrapper;  if so, push that copy 
      // onto the Lua stack. Otherwise, copy the passed-in wrapper into Lua and push it onto the Lua 
      // stack.
      //
      int push(lua_State*, const editor_script::wrapper&, const char* metatable_name);
      template<typename mt> inline int push(const editor_script::wrapper& instance) {
         static_assert(std::is_base_of_v<editor_script::wrapper_metatable, mt>);
         static_assert(!std::is_same_v<editor_script::wrapper_metatable, mt>);
         //
         return this->push(this->vm.lua_vm, instance, mt::metatable_key);
      }

      void remove_from_sequential_collection(editor_script::wrapper& to_remove);
};