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
   void _report_form_create_error(QWidget* window, dovah::form_creation_request::error_code ec) {
      using error_code = dovah::form_creation_request::error_code;
      //
      QString text;
      switch (ec) {
         case error_code::bad_form_type_requested:
            text = QObject::tr("An internal program error occurred: DovahKit tried to create a form but supplied a bad form type.");
            break;
         case error_code::no_active_file:
            text = QObject::tr("There is no active file, nor any room in the load order for a new file.");
            break;
         case error_code::no_form_id_available:
            text = QObject::tr("You've used up all of the form IDs available to this file!");
            break;
         case error_code::unsupported_form_type_requested:
            text = QObject::tr("DovahKit does not support editing this form type.");
            break;
         case error_code::invalid_parent_child_relationship:
            text = QObject::tr("The specified parent form cannot have a child form of this type. (Wait, what? How did you get the Object Window to try to do that?)");
            break;
         case error_code::exterior_grid_coordinates_already_taken:
            text = QObject::tr("The specified worldspace already has an exterior cell at the desired grid coordinates. (Wait, what? How did you get the Object Window to try and create an exterior cell?)");
            break;
      }
      QMessageBox::critical(
         window,
         QObject::tr("Error", "create new form error"),
         QObject::tr("Unable to create new form. %1").arg(text)
      );
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
      using error_code = dovah::form_creation_request::error_code;
      //
      auto form_types = this->ui.tree->selectedFormTypes();
      if (form_types.size() != 1)
         return;
      //
      auto& editor  = DovahKitCore::get();
      auto  request = editor.request_form_creation(form_types.back());
      if (request.get_error_code() != dovah::form_creation_request::error_code::none) {
         _report_form_create_error(this, request.get_error_code());
         return;
      }
      bool    ok        = false;
      QString editor_id = QInputDialog::getText(this, tr("Set editor ID"), tr("Editor ID:"), QLineEdit::Normal, "", &ok);
      if (!ok)
         return;
      request.editorID = editor_id.toStdString();
      request.commit();
      if (request.get_error_code() != dovah::form_creation_request::error_code::none) {
         _report_form_create_error(this, request.get_error_code());
      }
   });
   //
   this->_formActionEdit        = new QAction(tr("Edit...",     "object window form actions"), this->ui.table);
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
      auto* stub       = _get_selected_form(opener);
      auto  form_types = this->ui.tree->selectedFormTypes();
      this->_actionCreateForm->setEnabled(form_types.size() == 1);
      this->_formActionEdit->setVisible(stub != nullptr);
      this->_formActionShowUseInfo->setVisible(stub != nullptr);
      //
      QMenu menu(opener);
      menu.addAction(this->_actionCreateForm);
      menu.addAction(this->_formActionEdit);
      menu.addAction(this->_formActionShowUseInfo);
      //
      bool any = false;
      for (auto* action : menu.actions()) {
         if (action->isEnabled() && action->isVisible()) {
            any = true;
            break;
         }
      }
      if (!any)
         return; // don't show a menu if all of its contents are disaled or hidden
      //
      menu.exec(opener->mapToGlobal(pos));
   });
   #pragma endregion
}