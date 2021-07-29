#pragma once
#include <mutex>
#include <QObject>
#include "../../../../helpers/passkey.h"
#include "../coordinator/client_thread_script_borrow_handle.h"

struct ObservableStandardItemModelObserver;
namespace dovahscript::core::subsystems {
   class lifetime;
}

namespace dovahscript::impl {
   class lifetime_check_queue {
      public:
         using model_observer_t  = ObservableStandardItemModelObserver;
         using subsystem_passkey = cobb::passkey<lifetime_check_queue, core::subsystems::lifetime>;

      protected:
         std::mutex lock;
         struct {
            QVector<model_observer_t*> model_observers;
            QVector<QObject*> objects;
         } queues;
         core::client_thread_script_borrow_handle opportunity_handle;

         bool _empty() const noexcept; // doesn't lock

      public:
         void queue_check(model_observer_t&);
         void queue_check(QObject&);

         void main_thread_handler(subsystem_passkey);
         void on_script_teardown(subsystem_passkey);
   };
}
