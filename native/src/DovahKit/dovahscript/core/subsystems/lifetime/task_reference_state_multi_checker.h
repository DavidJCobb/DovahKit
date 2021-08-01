#pragma once

class ObservableStandardItemModelObserver;
class QObject;
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

      protected:
         core::subsystems::lifetime& lifetime_sys;
   };
}
