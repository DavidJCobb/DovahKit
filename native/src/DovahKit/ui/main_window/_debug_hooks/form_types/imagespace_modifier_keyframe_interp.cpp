#include "./imagespace_modifier_keyframe_interp.h"
#include <QInputDialog>
#include <QMessageBox>

#include "dovah/forms/ImagespaceModifier.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/core.h"
#include "ui/types/imagespace_modifier/computed_keyframe.h"
#include "ui/types/imagespace_modifier/keyframe.h"
#include "ui/types/imagespace_modifier/keyframe_collection.h"

namespace DovahKitDebug::features::form_types {
   /*static*/ void imagespace_modifier_keyframe_interp::execute(QWidget* from) {
      auto text = QInputDialog::getText(from, QObject::tr("Form ID?", "debug"), QObject::tr("ID:", "debug"));
      if (text.isEmpty())
         return;
      auto id = text.toInt(nullptr, 16);
      if (!id)
         return;
      auto* stub = DovahKitCore::get().get_form(id);
      if (!stub) {
         QMessageBox::critical(from, "Form not found", "Form not found");
         return;
      }
      if (stub->form_type != dovah::form_type::imagespace_modifier) {
         QMessageBox::critical(
            from,
            "Form is not an IMAD",
            QString("%1 is not an imagespace modifier.")
               .arg(editor_helpers::form_identifiers_to_string(stub))
         );
         return;
      }

      auto loaded = stub->load().ptr_cast<dovah::loaded_forms::ImagespaceModifier>();
      if (!loaded) {
         QMessageBox::critical(
            from,
            "Form load failed",
            QString("%1 could not be loaded into memory.")
               .arg(editor_helpers::form_identifiers_to_string(stub))
         );
         return;
      }

      ui::types::imagespace_modifier::keyframe_collection collection;
      collection.import_data(*loaded);

      float duration = loaded->duration;

      bool  ok = false;
      float timestamp = QInputDialog::getDouble(
         from,
         QString("Timestamp"),
         QString("Choose the timestamp to display. Range: [0, %1]").arg(duration),
         0,
         0,
         duration,
         3,
         &ok
      );
      if (!ok)
         return;

      auto computed = collection.get_computed_keyframe(timestamp, true);
      #if _DEBUG
         __debugbreak();
      #endif
   }
}