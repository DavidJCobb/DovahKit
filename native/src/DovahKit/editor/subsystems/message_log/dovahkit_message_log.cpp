#include "./dovahkit_message_log.h"
#include <QTimer>
#include "./model.h"

namespace dovahkit::subsystems::message_log {
   core::core() {
      this->_model = new class model(this);
      //
      // Defer initialization of the model (which depends on this subsystem) until after 
      // this subsystem is fully constructed.
      //
      QTimer::singleShot(0, [this]() {
         this->_model->initialize({});
      });

      QObject::connect(this->_model, &model::rowsInserted, this, [this](const QModelIndex&, int first, int last) {
         auto&  prior = this->_state.last_known_warning_count;
         size_t after = this->warning_count();
         if (after <= prior)
            return;
         size_t added = after - prior;
         prior = after;

         if (!this->_state.log_has_focus) {
            this->_state.unread_warnings += added;
         }

         emit this->warningCountsChanged(
            after,
            this->_state.unread_warnings
         );
      });
      QObject::connect(this->_model, &model::modelReset, this, [this]() {
         size_t after = this->warning_count();
         this->_state.last_known_warning_count = after;
         this->_state.unread_warnings = 0;
         emit this->warningCountsChanged(after, 0);
      });
   }
   core::~core() {
   }

   void core::set_log_ui_has_focus(bool v) {
      this->_state.log_has_focus = v;
      if (v) {
         auto& dst = this->_state.unread_warnings;
         if (dst > 0) {
            dst = 0;
            emit this->warningCountsChanged(this->warning_count(), 0);
         }
      }
   }

   size_t core::warning_count() const {
      return this->_model->warning_count();
   }
}