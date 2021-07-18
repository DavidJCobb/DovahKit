#include "messaging.h"
#include "editor_script_inner_core.h"
#include "../../../lua.h"

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
   bool blocking = m->is_blocking(); // grab this before adding it to the list, to avoid race conditions (e.g. the main thread executing and deleting a non-blocking task before we get a chance to check)
   vm.task_queues.s2m.push_back(m);
   if (blocking) {
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
   vm.ui_queues.read.push_back(&task);
   while (!task.seen)
      if (vm.is_aborted())
         break;
   if (!vm.is_aborted()) {
      vm.task_queues.m2s.urgent.process();
   }
}
void DovahKitScriptVMUITaskConduit::send_message(editor_script::cross_thread_task& task) {
   DovahKitScriptVMCore::require_script_thread();
   bool blocking = task.is_blocking(); // grab this before adding it to the list, to avoid race conditions (e.g. the main thread executing and deleting a non-blocking task before we get a chance to check)
   auto& vm = DovahKitScriptVMCore::get();
   vm.ui_queues.read.wait_until_empty();
   vm.ui_queues.write.push_back(&task);
   if (blocking) {
      while (!task.seen)
         if (vm.is_aborted())
            break;
   }
   if (!vm.is_aborted()) {
      vm.task_queues.m2s.urgent.process();
   }
}
#pragma endregion