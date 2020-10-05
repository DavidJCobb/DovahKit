#include "cell_view.h"
#include <QMenu>
#include "../../helpers/qt/strings.h"
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
   this->ui.filterFormType->setAllowUnfiltered(true);
   for (auto& info : dovah::form_types) {
      if (dovah::form_type_info::form_type_is_base_form(info.formType))
         this->ui.filterFormType->whitelistSignature(info.signature);
   }
   //
   this->ui.cellList->setWorldspacePicker(this->ui.worldspace);
   this->ui.referenceList->setCellPicker(this->ui.cellList);
   this->ui.referenceList->setTextFilter(this->ui.filterText);
   QObject::connect(this->ui.filterFormType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
      dovah::form_type_t ft = this->ui.filterFormType->formType();
      this->ui.referenceList->setFormTypeFilter(ft);
   });
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
   #pragma region Context menus
      #pragma region Cell
         this->cellContextMenu.edit        = new QAction(tr("Edit...",     "cell view cell actions"), this->ui.cellList);
         this->cellContextMenu.duplicate   = new QAction(tr("Duplicate",   "cell view cell actions"), this->ui.cellList);
         this->cellContextMenu.showUseInfo = new QAction(tr("Use Info...", "cell view cell actions"), this->ui.cellList);
         this->cellContextMenu.deleteForm  = new QAction(tr("Delete",      "cell view cell actions"), this->ui.cellList);
         QObject::connect(this->cellContextMenu.edit, &QAction::triggered, [this]() {
            if (auto* stub = this->ui.cellList->formStub())
               open_edit_dialog_for_form(stub, this->parentWidget());
         });
         QObject::connect(this->cellContextMenu.duplicate, &QAction::triggered, [this]() {
            if (auto* stub = this->ui.cellList->formStub())
               DovahKitCore::get().duplicate_form(*stub, this);
         });
         QObject::connect(this->cellContextMenu.showUseInfo, &QAction::triggered, [this]() {
            if (auto* stub = this->ui.cellList->formStub())
               open_use_info_dialog_for_form(stub, this->parentWidget());
         });
         QObject::connect(this->cellContextMenu.deleteForm, &QAction::triggered, [this]() {
            if (auto* stub = this->ui.cellList->formStub())
               DovahKitCore::get().delete_form(*stub, this);
         });
         //
         this->ui.cellList->setContextMenuPolicy(Qt::CustomContextMenu);
         QObject::connect(this->ui.cellList, &QWidget::customContextMenuRequested, [this](const QPoint& pos) {
            auto  opener = this->ui.cellList;
            auto& items  = this->cellContextMenu;
            if (!opener->formStub())
               return;
            //
            QMenu menu(opener);
            menu.addAction(items.edit);
            menu.addAction(items.duplicate);
            menu.addAction(items.showUseInfo);
            menu.addAction(items.deleteForm);
            menu.exec(opener->mapToGlobal(pos));
         });
      #pragma endregion
      #pragma region Reference
         this->refContextMenu.edit        = new QAction(tr("Edit...",     "cell view ref actions"), this->ui.referenceList);
         this->refContextMenu.duplicate   = new QAction(tr("Duplicate",   "cell view ref actions"), this->ui.referenceList);
         this->refContextMenu.showUseInfo = new QAction(tr("Use Info...", "cell view ref actions"), this->ui.referenceList);
         this->refContextMenu.deleteForm  = new QAction(tr("Delete",      "cell view ref actions"), this->ui.referenceList);
         QObject::connect(this->refContextMenu.edit, &QAction::triggered, [this]() {
            if (auto* stub = this->ui.referenceList->formStub())
               open_edit_dialog_for_form(stub, this->parentWidget());
         });
         QObject::connect(this->refContextMenu.duplicate, &QAction::triggered, [this]() {
            if (auto* stub = this->ui.referenceList->formStub())
               DovahKitCore::get().duplicate_form(*stub, this);
         });
         QObject::connect(this->refContextMenu.showUseInfo, &QAction::triggered, [this]() {
            if (auto* stub = this->ui.referenceList->formStub())
               open_use_info_dialog_for_form(stub, this->parentWidget());
         });
         QObject::connect(this->refContextMenu.deleteForm, &QAction::triggered, [this]() {
            if (auto* stub = this->ui.referenceList->formStub())
               DovahKitCore::get().delete_form(*stub, this);
         });
         //
         this->ui.referenceList->setContextMenuPolicy(Qt::CustomContextMenu);
         QObject::connect(this->ui.referenceList, &QWidget::customContextMenuRequested, [this](const QPoint& pos) {
            auto  opener = this->ui.referenceList;
            auto& items  = this->refContextMenu;
            if (!opener->formStub())
               return;
            //
            QMenu menu(opener);
            menu.addAction(items.edit);
            menu.addAction(items.duplicate);
            menu.addAction(items.showUseInfo);
            menu.addAction(items.deleteForm);
            menu.exec(opener->mapToGlobal(pos));
         });
      #pragma endregion
   #pragma endregion
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