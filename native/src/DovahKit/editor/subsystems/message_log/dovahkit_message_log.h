#pragma once
#include <QObject>
#include "helpers/singleton_ex.h"
#include "ui/types/logging/log_item.h"

namespace dovah::notices {
   class base_error;
   class base_warning;
}

namespace dovahkit::subsystems::message_log {
   class core;

   //
   // Subsystem for showing a log of warnings, errors, and other recent messages to 
   // the user.
   //
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      protected:
         core();
         ~core();

      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

         void addLogItem(const ui::types::log_item& item) {
            emit logItemReceived(item);
         }

      signals:
         void backendErrorReceived(const dovah::notices::base_error&);
         void backendWarningReceived(const dovah::notices::base_warning&);
         void logItemReceived(const ui::types::log_item&);
   };
};