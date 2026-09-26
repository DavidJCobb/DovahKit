#include "./uncaught_exception.h"

namespace DovahKitDebug::features::crash_tests {
   /*static*/ void uncaught_exception::execute(QWidget* from) {
      throw std::exception("Test exception, meant to be uncaught");
   }
}