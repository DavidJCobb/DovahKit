#include "send_script_task.h"
#include "core/subsystems/coordinator.h"

namespace dovahscript {
   extern void send_script_task(tasks::_base& task) {
      core::subsystems::coordinator::get().send_script_task(task);
   }
   extern void send_script_ui_task(tasks::_ui_read_base& task) {
      core::subsystems::coordinator::get().send_ui_read_task(task);
   }
   extern void send_script_ui_task(tasks::_ui_write_base& task) {
      core::subsystems::coordinator::get().send_ui_write_task(task);
   }
}
