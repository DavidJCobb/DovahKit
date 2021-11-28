#include "dk3d_debug_rebind.h"
#include <QMessageBox>
#include "../../../dk3d/DK3DInputHandler.h"

namespace DovahKitDebug::features {
   /*static*/ void dk3d_debug_rebind::execute(QWidget* window) {
      DK3DInputHandler::get().debugOpenBindEditWindow();
   }
}