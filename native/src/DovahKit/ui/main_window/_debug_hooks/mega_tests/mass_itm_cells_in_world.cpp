#include "./mass_itm_cells_in_world.h"
#include <QInputDialog>
#include <QMessageBox>
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/for_each_child_form.h"
#include "dovah/form_stubs/helpers/get_worldspace_persistent_cell.h"
#include "editor/core.h"

namespace DovahKitDebug::features::mega_tests {
   /*static*/ void mass_itm_cells_in_world::execute(QWidget* from) {
      auto text = QInputDialog::getText(from, QObject::tr("Worldspace form ID?", "debug"), QObject::tr("ID:", "debug"));
      if (text.isEmpty())
         return;
      auto id = text.toInt(nullptr, 16);
      if (!id)
         return;
      auto* stub = DovahKitCore::get().get_form(id);
      if (!stub) {
         QMessageBox::critical(from, "Error", "Form not found");
         return;
      }
      if (stub->form_type != dovah::form_type::worldspace) {
         QMessageBox::critical(from, "Error", "Form is not a worldspace");
         return;
      }

      auto* pcell = dovah::form_stub_helpers::get_worldspace_persistent_cell(*stub);
      dovah::form_stub_helpers::for_each_child_form(*stub, [pcell](dovah::form_stub& child) {
         if (child.form_type == dovah::form_type::cell && &child != pcell)
            child.set_edited(true);
         return false;
      });
      QMessageBox::information(from, "Done", QString("Flagged cells (besides persistent cell) as edited."));
   }
}