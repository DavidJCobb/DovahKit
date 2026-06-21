#include "./object_window.h"
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include "dovah/exceptions/form_creation_failed.h"
#include "dovah/exceptions/form_renumber_failed.h"
#include "editor/core.h"
#include "editor/open_window_for_form.h"
#include "editor/helpers/is_form_type_legal_to_create.h"
#include "editor/helpers/make_editor_id_for_duplicate.h"
#include "editor/localize/form_creation_error_code.h"
#include "./form_use_info.h"

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
   void _report_form_create_error(QWidget* window, const dovah::exceptions::form_creation_failed::error_code ec) {
      QString text = editor::localize::form_creation_error_code(ec);
      bool    should_be_impossible = false;
      switch (ec) {
         using enum dovah::exceptions::form_creation_failed::error_code;
         case invalid_form_type:
         case invalid_parent_child_relationship:
         case exterior_grid_coordinates_already_taken:
         case cannot_create_reference_with_no_parent_cell:
         case interior_cell_clone_cannot_have_parent:
         case exterior_cell_clone_must_have_parent:
            should_be_impossible = true;
            break;
      }
      if (should_be_impossible) {
         text = ObjectWindow::tr("%1 (Wait, what? How did you get the Object Window to try to do that?)").arg(text);
      }
      QMessageBox::critical(
         window,
         QObject::tr("Error", "create new form error"),
         QObject::tr("Unable to create new form. %1").arg(text)
      );
   }
   void _report_form_renumber_error(QWidget* window, const dovah::exceptions::form_renumber_failed& ex) {
      using error_code = std::decay_t<decltype(ex)>::error_code;

      QString text;
      switch (ex.code) {
         case error_code::form_is_not_from_active_file:
            text = QObject::tr("You can't renumber forms that do not originate from the active file.");
            break;
         case error_code::form_is_hardcoded:
            text = QObject::tr("You can't renumber hardcoded forms.");
            break;
         case error_code::form_id_is_in_hardcoded_range:
            text = QObject::tr("The desired form ID cannot be used; all IDs in the range xx000001 to xx0007FF are reserved for hardcoded forms.");
            break;
         case error_code::form_id_is_zero:
            text = QObject::tr("A form cannot have the form ID 00000000.");
            break;
         case error_code::form_id_is_out_of_bounds:
            text = QObject::tr("The desired form ID is out-of-bounds; its load order prefix places it outside of all loaded files.");
            break;
         case error_code::form_id_is_occupied:
            text = QObject::tr("The desired form ID is already in use.");
            break;
         case error_code::form_id_is_reserved:
            text = QObject::tr("DovahKit has reserved the desired form ID for use by some other process, such as the creation of a new form.");
            break;
         case error_code::no_active_file:
            text = QObject::tr("There is no active file, nor any room in the load order for a new file.");
            break;
         case error_code::cannot_sever_references_to_none_stub:
            text = QObject::tr("The desired ID is the target of one or more dangling references, and DovahKit does not know how to sever those references.");
            break;
         case error_code::cannot_inject_form_overtop_none_stub:
            text = QObject::tr("The desired ID is the target of one or more dangling references. These references are from forms defined outside of the active file, so DovahKit cannot sever them to make room for the injected form.");
            break;
      }
      QMessageBox::critical(
         window,
         QObject::tr("Error", "renumber form error"),
         QObject::tr("Unable to change this form's ID. %1").arg(text)
      );
   }
}

ObjectWindow::ObjectWindow(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   //
   this->ui.table->setSource(this->ui.tree);
   this->ui.table->setFilter(this->ui.filter);
   this->ui.table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
   QObject::connect(this->ui.table, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      auto* stub = _get_selected_form(this->ui.table);
      if (stub && !stub->is_none_stub())
         open_edit_dialog_for_form(*stub, this);
   });
   //
   #pragma region Context menu
   this->_actionCreateForm = new QAction(tr("New form...", "object window form actions"), this->ui.table);
   QObject::connect(this->_actionCreateForm, &QAction::triggered, this, [this]() {
      using exception  = dovah::exceptions::form_creation_failed;
      using error_code = exception::error_code;

      auto form_types = this->ui.tree->filterInfo().form_types;
      if (form_types.size() != 1)
         return;
      //
      dovah::form_stub* created_form = nullptr;
      try {
         auto& editor  = DovahKitCore::get();
         auto  request = editor.request_form_creation(form_types.back());
         {
            bool    ok = false;
            QString editor_id = QInputDialog::getText(this, tr("Set editor ID"), tr("Editor ID:"), QLineEdit::Normal, "", &ok);
            if (!ok)
               return;
            request.editorID = editor_id.toStdString();
         }
         created_form = request.commit();
      } catch (const exception& ex) {
         _report_form_create_error(this, ex.code);
      }
      if (created_form)
         this->ui.table->select(created_form);
   });
   //
   this->_formActionEdit        = new QAction(tr("Edit...",           "object window form actions"), this->ui.table);
   this->_formActionDuplicate   = new QAction(tr("Duplicate",         "object window form actions"), this->ui.table);
   this->_formActionShowUseInfo = new QAction(tr("Use Info...",       "object window form actions"), this->ui.table);
   this->_formActionRenumber    = new QAction(tr("Change form ID...", "object window form actions"), this->ui.table);
   this->_formActionDelete      = new QAction(tr("Delete",            "object window form actions"), this->ui.table);
   QObject::connect(this->_formActionEdit, &QAction::triggered, this, [this]() {
      auto* stub = _get_selected_form(this->ui.table);
      if (stub)
         open_edit_dialog_for_form(*stub, this->parentWidget());
   });
   QObject::connect(this->_formActionDuplicate, &QAction::triggered, this, [this]() {
      auto* stub = _get_selected_form(this->ui.table);
      if (!stub)
         return;
      
      dovah::form_stub* created_form = DovahKitCore::get().duplicate_form(*stub, this);
      if (created_form)
         this->ui.table->select(created_form);
   });
   QObject::connect(this->_formActionShowUseInfo, &QAction::triggered, this, [this]() {
      auto* stub = _get_selected_form(this->ui.table);
      if (stub)
         open_use_info_dialog_for_form(*stub, this->parentWidget());
   });
   QObject::connect(this->_formActionRenumber, &QAction::triggered, this, [this]() {
      auto* stub = _get_selected_form(this->ui.table);
      if (!stub)
         return;
      bool    ok   = false;
      QString text = QInputDialog::getText(this, tr("Choose form ID"), tr("What form ID do you want this form to use?"), QLineEdit::Normal, QString("%1").arg(stub->formID, 8, 16, QChar('0')).toUpper(), &ok);
      if (!ok)
         return;
      dovah::bare_form_id_t id = text.toUInt(&ok, 16);
      if (!ok) {
         QMessageBox::critical(
            this,
            QObject::tr("Error", "renumber form error"),
            QObject::tr("\"%1\" is not a valid form ID. A form ID is an eight-digit hexadecimal number (that is, each digit is between 0-9 or A-F, inclusive).").arg(text)
         );
         return;
      }
      if (id == stub->formID) {
         QMessageBox::critical(
            this,
            QObject::tr("Error", "renumber form error"),
            QObject::tr("That form's ID is already %1.").arg(QString("%1").arg(id, 8, 16, QChar('0')).toUpper())
         );
         return;
      }
      //
      auto& editor  = DovahKitCore::get();
      try {
         auto request = editor.request_form_renumber(*stub, id);
         request.commit();
      } catch (const dovah::exceptions::form_renumber_failed& ex) {
         _report_form_renumber_error(this, ex);
         return;
      }
      this->ui.table->select(stub);
   });
   QObject::connect(this->_formActionDelete, &QAction::triggered, this, [this]() {
      auto* stub = _get_selected_form(this->ui.table);
      if (!stub)
         return;
      DovahKitCore::get().delete_form(*stub, this);
   });
   //
   this->ui.table->setContextMenuPolicy(Qt::CustomContextMenu);
   QObject::connect(this->ui.table, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
      auto opener = this->ui.table;
      //
      auto* stub       = _get_selected_form(opener);
      auto  form_types = this->ui.tree->filterInfo().form_types;
      //
      bool form_exists  = (stub != nullptr);
      bool is_alterable = form_exists && !stub->is_none_stub();
      //
      this->_actionCreateForm->setEnabled(form_types.size() == 1 && form_types[0] != dovah::form_type::none);
      this->_formActionDuplicate->setEnabled(true);
      this->_formActionEdit->setVisible(is_alterable);
      this->_formActionDuplicate->setVisible(is_alterable);
      this->_formActionShowUseInfo->setVisible(form_exists);
      this->_formActionRenumber->setVisible(is_alterable);
      this->_formActionRenumber->setEnabled(is_alterable && DovahKitCore::get().is_form_defined_in_active_file(stub));
      this->_formActionDelete->setVisible(is_alterable);
      if (form_types.size() == 1) {
         auto type = form_types[0];
         if (!editor_helpers::is_form_type_legal_to_create(type)) {
            this->_actionCreateForm->setEnabled(false);
         }
      }
      if (stub) {
         if (!editor_helpers::is_form_type_legal_to_create(stub->form_type)) {
            this->_formActionDuplicate->setEnabled(false);
         }
      }
      //
      QMenu menu(opener);
      menu.addAction(this->_actionCreateForm);
      menu.addAction(this->_formActionEdit);
      menu.addAction(this->_formActionDuplicate);
      menu.addAction(this->_formActionShowUseInfo);
      menu.addAction(this->_formActionRenumber);
      menu.addAction(this->_formActionDelete);
      //
      if (menu.isEmpty())
         return; // don't show a menu if all of its contents are disabled or hidden
      menu.exec(opener->mapToGlobal(pos));
   });
   #pragma endregion
}