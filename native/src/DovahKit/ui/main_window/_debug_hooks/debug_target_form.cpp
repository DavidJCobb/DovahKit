#include "debug_target_form.h"
#include <QInputDialog>
#include "../../../dovah/form_stub.h"
#include "../../../editor/core.h"

namespace DovahKitDebug {
   extern void debug_target_form(QWidget* window) {
      auto path = QInputDialog::getText(window, QObject::tr("Form ID?", "debug"), QObject::tr("ID:"));
      if (path.isEmpty())
         return;
      auto id = path.toInt(nullptr, 16);
      if (!id)
         return;
      auto* stub = DovahKitCore::get().get_form(id);
      if (stub) {
         auto form = stub->load();
         __debugbreak();
      } else {
         __debugbreak(); // no stub
      }
   }
}