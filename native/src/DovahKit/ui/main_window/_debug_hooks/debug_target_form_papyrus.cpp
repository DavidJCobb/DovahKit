#include "debug_target_form_papyrus.h"
#include <QInputDialog>
#include "../../../dovah/form_stub.h"
#include "../../../dovah/forms/Form.h"
#include "../../../editor/core.h"

namespace DovahKitDebug {
   extern void debug_target_form_papyrus(QWidget* window) {
      auto path = QInputDialog::getText(window, QObject::tr("Form ID?", "debug"), QObject::tr("ID:"));
      if (path.isEmpty())
         return;
      auto id = path.toInt(nullptr, 16);
      if (!id)
         return;
      auto* stub = DovahKitCore::get().get_form(id);
      if (stub) {
         auto form = stub->load();
         if (form) {
            auto* papyrus = form->get_papyrus_data();
            __debugbreak(); // papyrus data
            return;
         }
      }
      __debugbreak(); // no stub, or form can't load, or no papyrus data on this form type
   }
}