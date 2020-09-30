#include "object_window.h"
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include "../../editor/core.h"
#include "../../editor/open_window_for_form.h"
#include "form_use_info.h"

namespace {
   dovah::form_stub* _get_selected_form(QTableView* widget) {
      auto* proxy  = (QSortFilterProxyModel*)widget->model();
      auto  select = widget->selectionModel()->selection().indexes();
      if (!select.size())
         return nullptr;
      auto  real   = proxy->mapToSource(select[0]); // the (index) we received is specific to the proxy; we need an index relative to the underlying model
      if (!real.isValid())
         return nullptr;
      using item_type = FormTable::model_item_type;
      //
      auto data = (item_type*)real.internalPointer();
      if (!data)
         return nullptr;
      return data->stub;
   }
}

ObjectWindow::ObjectWindow(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   //
   this->ui.table->setSource(this->ui.tree);
   this->ui.table->setFilter(this->ui.filter);
   QObject::connect(this->ui.table, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      auto* stub = _get_selected_form(this->ui.table);
      if (stub)
         open_edit_dialog_for_form(stub, this);
   });
   //
   #pragma region Context menu
   this->_actionCreateForm = new QAction(tr("New form...", "object window form actions"), this->ui.table);
   QObject::connect(this->_actionCreateForm, &QAction::triggered, this, [this]() {
      auto form_types = this->ui.tree->selectedFormTypes();
      if (form_types.size() != 1)
         return;
      //
      auto& editor = DovahKitCore::get();
      auto* stub   = editor.create_form_of_type(form_types.back());
      if (!stub) {
         QMessageBox::critical(
            this,
            tr("Error", "create new form error"),
            tr("Unable to create new form.")
         );
         return;
      }
      bool    ok        = false;
      QString editor_id = QInputDialog::getText(this, tr("Set editor ID"), tr("Editor ID:"), QLineEdit::Normal, stub->get_editor_id(), &ok);
      if (!ok) {
         //
         // TODO: delete the form? or don't bother?
         //
      }
      stub->editorID = editor_id.toStdString();
      emit editor.formModified(stub);
   });
   //
   this->_formActionEdit        = new QAction(tr("Edit...", "object window form actions"), this->ui.table);
   this->_formActionShowUseInfo = new QAction(tr("Use Info...", "object window form actions"), this->ui.table);
   QObject::connect(this->_formActionEdit, &QAction::triggered, this, [this]() {
      auto* stub = _get_selected_form(this->ui.table);
      if (stub)
         open_edit_dialog_for_form(stub, this->parentWidget());
   });
   QObject::connect(this->_formActionShowUseInfo, &QAction::triggered, this, [this]() {
      auto* stub = _get_selected_form(this->ui.table);
      if (stub)
         open_use_info_dialog_for_form(stub, this->parentWidget());
   });
   //
   this->ui.table->setContextMenuPolicy(Qt::CustomContextMenu);
   QObject::connect(this->ui.table, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
      auto opener = this->ui.table;
      //
      auto form_types = this->ui.tree->selectedFormTypes();
      this->_actionCreateForm->setEnabled(form_types.size() == 1);
      //
      QMenu menu(opener);
      menu.addAction(this->_actionCreateForm);
      menu.addAction(this->_formActionEdit);
      menu.addAction(this->_formActionShowUseInfo);
      menu.exec(opener->mapToGlobal(pos));
   });
   #pragma endregion
}