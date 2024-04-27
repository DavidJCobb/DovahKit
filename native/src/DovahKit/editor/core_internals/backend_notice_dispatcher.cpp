#include "./backend_notice_dispatcher.h"
#include "dovah/notices/base_error.h"
#include "dovah/notices/base_warning.h"

namespace DovahKitEditorInternals {
   void backend_notice_dispatcher::send(const dovah::notices::base_error& notice) {
      emit this->receivedError(notice.clone());
   }
   void backend_notice_dispatcher::send(const dovah::notices::base_warning& notice) {
      emit this->receivedWarning(notice.clone());
   }
}