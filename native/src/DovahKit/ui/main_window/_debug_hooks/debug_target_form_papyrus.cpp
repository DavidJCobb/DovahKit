#include "debug_target_form_papyrus.h"
#include <QInputDialog>
#include "../../../dovah/form_stub.h"
#include "../../../dovah/forms/Form.h"
#include "../../../editor/core.h"

namespace DovahKitDebug::features {
   /*static*/ void debug_target_form_papyrus::execute(QWidget* window) {
      auto text = QInputDialog::getText(window, QObject::tr("Form ID?", "debug"), QObject::tr("ID:", "debug"));
      if (text.isEmpty())
         return;
      auto id = text.toInt(nullptr, 16);
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