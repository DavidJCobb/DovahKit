#include "cell_view.h"
#include <type_traits>
#include <QMenu>
#include "helpers/qt/strings.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#include "editor/open_window_for_form.h"
#include "../generic/FormsOfTypeCombobox.h"
#include "editor/subsystems/worldedit.h"

CellViewWindow::CellViewWindow(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   auto& worldedit = dovahkit::subsystems::worldedit::get_or_create(); // ensure the subsystem exists
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
      //
      // When Cell View's selected cell changes, display its name as a heading above 
      // the list of refs in the selected cell.
      //
      auto    widget = this->ui.selectedCellName;
      QString text;
      if (cell) {
         text = cell->get_editor_id();
         if (text.isEmpty())
            text = tr("<i>Unnamed Cell</i>", "cell view");
         //
         if (cell->is_exterior_cell()) {
            text = tr("%1 (%2, %3)").arg(text);
            //
            int32_t x;
            int32_t y;
            if (cell->get_grid_coordinates(x, y)) {
               text = text.arg(x).arg(y);
            } else {
               text = text.arg("?").arg("?");
            }
         }
      } else {
         text = tr("No Cell Selected", "cell view");
      }
      widget->setText(text);
   });
   QObject::connect(this->ui.cellList, &CellList::renderRequested, this, [this](dovah::form_stub* cell) {
      dovahkit::subsystems::worldedit::get().set_current_area(cell);
   });
   //
   QObject::connect(this->ui.loadedCellsAtTop, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.cellList->setLoadedCellsAtTop(checked);
   });
   QObject::connect(this->ui.worldspace, &FormsOfTypeCombobox::formChanged, this, [this](dovah::form_stub* stub) {
      if (!DovahKitCore::get().has_data())
         return;
      this->ui.jumpToGrid->setEnabled(stub != nullptr);
   });
   QObject::connect(this->ui.jumpToGrid, &QPushButton::clicked, this, [this]() {
      auto* world = this->ui.worldspace->formStub();
      if (!world)
         return;
      dovahkit::subsystems::worldedit::get().set_current_area(
         world,
         this->ui.jumpToGridX->value(),
         this->ui.jumpToGridY->value()
      );
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
   #pragma region Synchronize Cell View and Worldedit selections
   {
      // ensure we don't cause Qt signal feedback loops with Cell View triggering 
      // changes to Worldedit triggering changes to Cell View triggering changes 
      // to Worldedit...
      static bool is_synchronizing = false;
      //
      // VOCABULARY:
      // 
      // peek
      //    Using Cell View to view the list of refs in a cell that isn't loaded 
      //    in Worldedit (i.e. a cell that isn't in the Render Window).
      // 
      // -------------------------------------------------------------------------
      // 
      // CASE 1:
      // Worldedit has refs selected in Cell A. We peeked Cell B, and then brought 
      // Cell View back to Cell A. We should select, in Cell View, all refs that 
      // are selected within Worldedit.
      //
      QObject::connect(this->ui.cellList, &CellList::currentCellChanged, this, [this](const dovah::form_stub* cell) {
         if (is_synchronizing)
            return;
         auto& worldedit = dovahkit::subsystems::worldedit::get();
         if (!worldedit.is_cell_loaded(cell))
            return;
         auto* sel_model = this->ui.referenceList->selectionModel();
         sel_model->clear();
         //
         is_synchronizing = true;
         for (auto* stub : this->ui.referenceList->formStubs()) {
            if (worldedit.is_ref_selected(stub)) {
               this->ui.referenceList->selectStub(stub, QItemSelectionModel::SelectionFlag::Select);
            }
         }
         is_synchronizing = false;
      });
      //
      // CASE 2:
      // We peeked Cell B while Cell A was loaded in Worldedit. Worldedit was then 
      // used to load Cell B. We should select, in Worldedit, all refs that are 
      // selected within Cell View.
      //
      QObject::connect(&worldedit, &std::decay_t<decltype(worldedit)>::cellLoaded, [this](dovah::form_stub& cell) {
         if (is_synchronizing)
            return;
         auto* window_cell = this->ui.cellList->formStub();
         if (window_cell != &cell)
            return;
         is_synchronizing = true;
         auto& worldedit = dovahkit::subsystems::worldedit::get();
         for (auto* stub : this->ui.referenceList->selectedStubs()) {
            worldedit.setRefSelectionState(*stub, true);
         }
         is_synchronizing = false;
      });
      //
      // CASE 3:
      // The selection is modified within Cell View.
      //
      QObject::connect(this->ui.referenceList, &CellRefList::selectionChanged, [this](const auto& selected, const auto& deselected) {
         if (is_synchronizing)
            return;
         auto& worldedit = dovahkit::subsystems::worldedit::get();
         if (!selected.empty()) {
            auto* window_cell = this->ui.cellList->formStub();
            if (!worldedit.is_cell_loaded(window_cell)) {
               //
               // CASE 3a:
               // The selection in Cell View is modified while peeking a cell that 
               // isn't loaded in Worldeit. We should wholly replace Worldedit's 
               // selection.
               //
               is_synchronizing = true;
               worldedit.deselectAllRefs();
               is_synchronizing = false;
               return;
            }
         }
         //
         // CASE 3b:
         // The selection in Cell View is modified while it's listing the refs in 
         // a cell that Worldedit has loaded. We should synchronize changes to the 
         // selection -- selections and deselections.
         //
         is_synchronizing = true;
         for(auto* stub : selected)
            if (stub)
               worldedit.setRefSelectionState(*stub, true);
         for (auto* stub : deselected)
            if (stub)
               worldedit.setRefSelectionState(*stub, false);
         is_synchronizing = false;
      });
      //
      // CASE 4:
      // The selection is modified within Worldedit.
      //
      QObject::connect(&worldedit, &std::decay_t<decltype(worldedit)>::refSelectionChanged, [this](dovah::form_stub& refr, bool selected) {
         if (is_synchronizing)
            return;
         auto* cell = refr.get_parent_form();
         if (!cell)
            return;
         auto* window_cell = this->ui.cellList->formStub();
         if (window_cell != cell) {
            //
            // CASE 4a:
            // The selection in Worldedit is modified while peeking a cell that 
            // isn't loaded in Worldeit. We should clear Cell View's selection.
            //
            this->ui.referenceList->selectStub(nullptr, QItemSelectionModel::SelectionFlag::Clear);
            return;
         }
         //
         // CASE 4b:
         // The selection in Worldedit is modified while Cell View is listing the 
         // refs in a cell that Worldedit has loaded. We should synchronize this 
         // selection change into Cell View.
         //
         is_synchronizing = true;
         this->ui.referenceList->selectStub(&refr, selected ? QItemSelectionModel::SelectionFlag::Select : QItemSelectionModel::SelectionFlag::Deselect);
         is_synchronizing = false;
      });
      //
      // CASE 5:
      // Cell View's ref list is filtered (by form type or by name). Worldedit's 
      // selection is changed to include a ref currently filtered out from display 
      // in Cell View. Then, Cell View is unfiltered. We need to select the ref at 
      // that time.
      //
      QObject::connect(this->ui.referenceList, &CellRefList::filterChanged, [this]() {
         auto& worldedit = dovahkit::subsystems::worldedit::get();
         if (!worldedit.is_cell_loaded(this->ui.cellList->formStub()))
            return;
         auto list = worldedit.get_selected_refs();
         
         is_synchronizing = true;
         this->ui.referenceList->selectStub(nullptr, QItemSelectionModel::SelectionFlag::Clear);
         for(auto* stub : list)
            this->ui.referenceList->selectStub(stub, QItemSelectionModel::SelectionFlag::Select);
         is_synchronizing = false;
      });
   }
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
   this->ui.jumpToGrid->setEnabled(this->ui.worldspace->formStub() != nullptr);
   this->ui.filterFormType->setEnabled(state);
   this->ui.filterText->setEnabled(state);
   this->ui.loadedCellsAtTop->setEnabled(state);
   this->ui.cellList->setEnabled(state);
   this->ui.referenceList->setEnabled(state);
}