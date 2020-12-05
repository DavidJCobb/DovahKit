#include "detailed_notice_dispatcher.h"

namespace DovahKitEditorInternals {
   detailed_notice_dispatcher::detailed_notice_dispatcher() {
      qRegisterMetaType<DovahKitEditorInternals::multithreadable_detailed_notice>(); // needed so that QObject::connect can pass these across threads (by copying them)
   }
   void detailed_notice_dispatcher::send(const dovah::detailed_notice& warning) {
      multithreadable_detailed_notice mt;
      mt.warning = warning;
      //
      emit this->received(mt);
   }
}