#include "task_reference.h"
#include "core/subsystems/lifetime.h"
#include "core/subsystems/resources/DovahscriptResource.h"

namespace dovahscript::impl::task_reference {
   extern void inc(DovahscriptResource* o) {
      if (!o)
         return;
      o->_on_task_referenced();
   }
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

   extern void dec(DovahscriptResource* o) {
      if (!o)
         return;
      o->_on_task_unreferenced();
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

   extern DovahscriptResource* strip(const DovahscriptResourceHandle& o) {
      return (DovahscriptResource*)o;
   }
}