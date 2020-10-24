#include "read_warning_dispatcher.h"

namespace DovahKitEditorInternals {
   read_warning_dispatcher::read_warning_dispatcher() {
      qRegisterMetaType<DovahKitEditorInternals::multithreadable_file_read_warning>(); // needed so that QObject::connect can pass these across threads (by copying them)
   }
   void read_warning_dispatcher::send(const dovah::file_read_warning& warning) {
      multithreadable_file_read_warning mt;
      mt.warning = warning;
      //
      emit this->received(mt);
   }
}