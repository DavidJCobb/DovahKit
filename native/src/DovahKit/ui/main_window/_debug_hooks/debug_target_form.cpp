#include "debug_target_form.h"
#include <QInputDialog>
#include "../../../dovah/form_stub.h"
#include "../../../editor/core.h"

namespace DovahKitDebug::features {
   /*static*/ void debug_target_form::execute(QWidget* window) {
      auto text = QInputDialog::getText(window, QObject::tr("Form ID?", "debug"), QObject::tr("ID:", "debug"));
      if (text.isEmpty())
         return;
      auto id = text.toInt(nullptr, 16);
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