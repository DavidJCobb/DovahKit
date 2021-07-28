#include "task_reference.h"
#include "core/subsystems/lifetime.h"

namespace dovahscript::impl::task_reference {
   extern void inc(QObject* o) {
      if (!o)
         return;
      core::subsystems::lifetime::get().add_task_reference(o);
   }
   extern void inc(ObservableStandardItemModelObserver* o) {
      if (!o)
         return;
      core::subsystems::lifetime::get().add_task_reference(o);
   }

   extern void dec(QObject* o) {
      if (!o)
         return;
      core::subsystems::lifetime::get().remove_task_reference(o);
      }
   extern void dec(ObservableStandardItemModelObserver* o) {
      if (!o)
         return;
      core::subsystems::lifetime::get().remove_task_reference(o);
   }
}