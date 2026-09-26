#include "./access_violation.h"
#include <QMessageBox>

namespace DovahKitDebug::features::crash_tests {
   /*static*/ void access_violation::execute(QWidget* from) {
      volatile int v = *(int*)-1;

      // ensure the variable is used
      QMessageBox::information(from, "This should never appear", QString("Deferenced int: %1").arg(v));
   }
}