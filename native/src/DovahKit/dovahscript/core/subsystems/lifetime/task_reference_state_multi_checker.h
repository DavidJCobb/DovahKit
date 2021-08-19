#pragma once

class  QObject;
struct ObservableStandardItemModelObserver;
namespace dovahscript::core::subsystems {
   class lifetime;
}

namespace dovahscript::impl {
   class task_reference_state_multi_checker {
      public:
         task_reference_state_multi_checker();
         ~task_reference_state_multi_checker();

         bool is_task_referenced(ObservableStandardItemModelObserver&) const noexcept;
         bool is_task_referenced(QObject&) const noexcept;

         // I would've liked to just have this class manage the mutex on the lifetime singleton 
         // via RAII, but unfortunately, constructing an instance of this class can involve 
         // several automatic copies, which can result in the (non-recursive) mutex deadlocking.
         void set_active_state(bool b);

      protected:
         core::subsystems::lifetime& lifetime_sys;
         bool is_active = false;
   };
}
