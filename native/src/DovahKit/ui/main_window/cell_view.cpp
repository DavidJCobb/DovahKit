#include "cell_view.h"
#include <QMenu>
#include "../../dovah/form_stub.h"
#include "../../editor/core.h"
#include "../../editor/open_window_for_form.h"
#include "../generic/FormsOfTypeCombobox.h"

CellViewWindow::CellViewWindow(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   //
   this->ui.worldspace->addFormType(dovah::form_type::worldspace);
   this->ui.worldspace->setNoneLabel(tr(" Interiors", "worldspace selector"));
   this->ui.worldspace->setAllowNone(true);
   //
   this->ui.cellList->setWorldspacePicker(this->ui.worldspace);
   this->ui.referenceList->setCellPicker(this->ui.cellList);
   QObject::connect(this->ui.cellList, &CellList::currentCellChanged, this, [this](const dovah::form_stub* cell) {
      auto    widget = this->ui.selectedCellName;
      QString text;
      if (cell) {
         text = cell->get_editor_id();
         if (text.isEmpty())
            text = tr("<i>Unnamed Cell</i>", "cell view");
         //
         if (cell->groupInfo.parentFormID)
            text = tr("%1 (%2, %3)").arg(text).arg(cell->groupInfo.gridX).arg(cell->groupInfo.gridY);
      } else {
         text = tr("No Cell Selected", "cell view");
      }
      widget->setText(text);
   });
   //
   this->setAllEnableStates(false);
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
      this->ui.worldspace->populate();
      this->ui.cellList->rebuildModel();
      this->ui.referenceList->rebuildModel();
      this->setAllEnableStates(true);
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->setAllEnableStates(false);
      this->ui.selectedCellName->setText(tr("No Cell Selected", "cell view"));
   });
}
void CellViewWindow::setAllEnableStates(bool state) {
   this->ui.worldspace->setEnabled(state);
   this->ui.jumpToGrid->setEnabled(state);
   this->ui.filterFormType->setEnabled(state);
   this->ui.filterText->setEnabled(state);
   this->ui.loadedCellsAtTop->setEnabled(state);
   this->ui.cellList->setEnabled(state);
   this->ui.referenceList->setEnabled(state);
}