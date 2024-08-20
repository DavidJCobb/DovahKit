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
   class model;

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

         void set_log_ui_has_focus(bool);

         size_t warning_count() const;
         constexpr size_t unread_warning_count() const { return this->_state.unread_warnings; }

         constexpr model* model() const { return this->_model; }

      signals:
         void backendErrorReceived(const dovah::notices::base_error&);
         void backendWarningReceived(const dovah::notices::base_warning&);
         void logItemReceived(const ui::types::log_item&);
         void warningCountsChanged(size_t all, size_t unread);

      protected:
         class model* _model = nullptr;
         struct {
            bool   log_has_focus   = false;
            size_t unread_warnings = 0;
            size_t last_known_warning_count = 0;
         } _state;
   };
};