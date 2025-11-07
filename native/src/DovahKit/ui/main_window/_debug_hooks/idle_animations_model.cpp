#include "idle_animations_model.h"
#include "ui/form_group_windows/idle/IdleAnimationsDialog.h"

namespace DovahKitDebug::features {
   /*static*/ void idle_animations_model::execute(QWidget* from) {
      //
      // TODO: eventually this window should be:
      // 
      //  * accessible via the main window's menu bar
      // 
      //  * accessible via anything that would open an IDLE form
      // 
      //  * tracked and reused, so you can't have more than one of it
      //
      auto* dialog = new IdleAnimationsDialog(from);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      dialog->show();
   }
}
