#include "messaging.h"
#include "editor_script_inner_core.h"
#include "../../../../Lua/lua.hpp"

namespace {
   // If we expect certain cross-thread tasks to be sent in large quantities, and if later tasks of a 
   // given type make earlier tasks redundant, then we can use the tasks' "collapse keys" to delete 
   // the older tasks when a newer task is received, if the task queue has a large number of items in 
   // it. This hasn't yet been needed, though.
   static constexpr bool use_task_collapse_keys = false;
}

#pragma region DovahKitScriptVMMessenger
void DovahKitScriptVMMessenger::send_message(editor_script::cross_thread_task* m) {
   DovahKitScriptVMCore::require_script_thread();
   auto& vm = DovahKitScriptVMCore::get();
   {
      auto  guard = std::lock_guard(vm.task_queues.s2m.lock);
      auto& list  = vm.task_queues.s2m.list;
      list.push_back(m);
   }
   if (m->is_blocking()) {
      while (!m->seen)
         if (vm.is_aborted())
            break;
      if (!vm.is_aborted()) {
         vm.task_queues.m2s.urgent.process();
      }
      if (vm.is_running() && vm.is_aborted()) {
         luaL_error(vm.lua_vm, "Script terminated at the user's request.");
         __assume(0); // luaL_error performs a jump and so does not return
      }
   }
}
#pragma endregion 

#pragma region DovahKitScriptVMUITaskConduit
void DovahKitScriptVMUITaskConduit::send_message(editor_script::ui_read_task& task) {
   DovahKitScriptVMCore::require_script_thread();
   auto& vm = DovahKitScriptVMCore::get();
   vm.ui_queues.write.wait_until_empty();
   {
      auto  guard = std::lock_guard(vm.ui_queues.read.lock);
      auto& list = vm.ui_queues.read.list;
      list.push_back(&task);
   }
   while (!task.seen)
      if (vm.is_aborted())
         break;
   if (!vm.is_aborted()) {
      vm.task_queues.m2s.urgent.process();
   }
   if (vm.is_running() && vm.is_aborted()) {
      luaL_error(vm.lua_vm, "Script terminated at the user's request.");
      __assume(0); // luaL_error performs a jump and so does not return
   }
}
void DovahKitScriptVMUITaskConduit::send_message(editor_script::cross_thread_task& task) {
   DovahKitScriptVMCore::require_script_thread();
   auto& vm = DovahKitScriptVMCore::get();
   vm.ui_queues.read.wait_until_empty();
   //
   bool blocking = task.is_blocking(); // grab this before adding it to the list, to avoid race conditions (e.g. the main thread executing and deleting a non-blocking task before we get a chance to check)
   {
      auto  guard = std::lock_guard(vm.ui_queues.write.lock);
      auto& list = vm.ui_queues.write.list;
      //
      if (use_task_collapse_keys && !blocking && task.collapse_key && list.size() > 1000) {
         std::vector<size_t> remove;
         //
         auto size = list.size();
         for (size_t i = 0; i < size; ++i) {
            auto*& t = list[i];
            assert(t);
            if (t->collapse_key == task.collapse_key) {
               if (!t->is_blocking() && t->is_fire_and_forget())
                  delete t;
               t = nullptr;
            }
         }
         list.erase(std::remove(list.begin(), list.end(), nullptr), list.end());
      }
      //
      list.push_back(&task);
   }
   if (blocking) {
      while (!task.seen)
         if (vm.is_aborted())
            break;
      if (!vm.is_aborted()) {
         vm.task_queues.m2s.urgent.process();
      }
      if (vm.is_running() && vm.is_aborted()) {
         luaL_error(vm.lua_vm, "Script terminated at the user's request.");
         __assume(0); // luaL_error performs a jump and so does not return
      }
   }
}
#pragma endregion