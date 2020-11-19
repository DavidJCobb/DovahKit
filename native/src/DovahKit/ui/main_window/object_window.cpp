#include "object_window.h"
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include "../../dovah/notice_code_list.h"
#include "../../editor/core.h"
#include "../../editor/open_window_for_form.h"
#include "../../editor/helpers/make_editor_id_for_duplicate.h"
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
   void _report_form_create_error(QWidget* window, dovah::notice_code_t ec) {
      using notice_code = dovah::notice_code;
      //
      QString text;
      switch (ec) {
         case notice_code::unknown_form_type:
            text = QObject::tr("An internal program error occurred: DovahKit tried to create a form but supplied a bad form type.");
            break;
         case notice_code::no_active_file:
            text = QObject::tr("There is no active file, nor any room in the load order for a new file.");
            break;
         case notice_code::form_id_unavailable_for_game_setting:
            text = QObject::tr("You've used up all of the form IDs available to this file!");
            break;
         case notice_code::unimplemented_form_type:
            text = QObject::tr("DovahKit does not support editing this form type.");
            break;
         case notice_code::invalid_parent_child_relationship:
            text = QObject::tr("The specified parent form cannot have a child form of this type. (Wait, what? How did you get the Object Window to try to do that?)");
            break;
         case notice_code::exterior_grid_coordinates_already_taken:
            text = QObject::tr("The specified worldspace already has an exterior cell at the desired grid coordinates. (Wait, what? How did you get the Object Window to try and create an exterior cell?)");
            break;
         case notice_code::cannot_create_reference_with_no_parent_cell:
            text = QObject::tr("References cannot be created outside of a cell. (Wait, what? How did you get the Object Window to try and create a reference?)");
            break;
         case notice_code::interior_cell_clone_cannot_have_parent:
            text = QObject::tr("Interior cells cannot have a parent worldspace. (Wait, what? How did you get the Object Window to try and create an interior cell?)");
            break;
         case notice_code::exterior_cell_clone_must_have_parent:
            text = QObject::tr("Exterior cells must have a parent worldspace. (Wait, what? How did you get the Object Window to try and create an exterior cell?)");
            break;
         case notice_code::cannot_sever_references_to_none_stub:
            text = QObject::tr("DovahKit needed to select a form ID to use for the new form. The chosen form ID is the target of one or more dangling references, and DovahKit does not know how to sever those references, so the form creation process could not continue.");
            break;
      }
      QMessageBox::critical(
         window,
         QObject::tr("Error", "create new form error"),
         QObject::tr("Unable to create new form. %1").arg(text)
      );
   }
   void _report_form_renumber_error(QWidget* window, dovah::notice_code_t ec) {
      using notice_code = dovah::notice_code;
      //
      QString text;
      switch (ec) {
         case notice_code::form_is_not_defined_in_active_file:
            text = QObject::tr("You can't renumber forms that do not originate from the active file.");
            break;
         case notice_code::cannot_renumber_hardcoded_form:
            text = QObject::tr("You can't renumber hardcoded forms.");
            break;
         case notice_code::form_id_is_in_the_hardcoded_range:
            text = QObject::tr("The desired form ID cannot be used; all IDs in the range xx000001 to xx0007FF are reserved for hardcoded forms.");
            break;
         case notice_code::zero_is_not_an_allowed_form_id:
            text = QObject::tr("A form cannot have the form ID 00000000.");
            break;
         case notice_code::form_id_is_out_of_bounds:
            text = QObject::tr("The desired form ID is out-of-bounds; its load order prefix places it outside of all loaded files.");
            break;
         case notice_code::form_id_is_already_in_use:
            text = QObject::tr("The desired form ID is already in use.");
            break;
         case notice_code::form_id_is_reserved_for_other_process:
            text = QObject::tr("DovahKit has reserved the desired form ID for use by some other process, such as the creation of a new form.");
            break;
         case notice_code::cannot_load_all_users_of_this_form:
            text = QObject::tr("DovahKit cannot renumber a form unless it supports editing all of the forms that use the target form.");
            break;
         case notice_code::no_active_file:
            text = QObject::tr("There is no active file, nor any room in the load order for a new file.");
            break;
         case notice_code::cannot_sever_references_to_none_stub:
            text = QObject::tr("The desired ID is the target of one or more dangling references, and DovahKit does not know how to sever those references.");
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
      auto& editor  = DovahKitCore::get();
      auto  request = editor.request_form_creation(form_types.back());
      if (request.get_error_code() != dovah::default_notice_code) {
         _report_form_create_error(this, request.get_error_code());
         return;
      }
      bool    ok        = false;
      QString editor_id = QInputDialog::getText(this, tr("Set editor ID"), tr("Editor ID:"), QLineEdit::Normal, "", &ok);
      if (!ok)
         return;
      request.editorID = editor_id.toStdString();
      auto result = request.commit();
      if (request.get_error_code() != dovah::default_notice_code) {
         _report_form_create_error(this, request.get_error_code());
      }
      if (result)
         this->ui.table->select(result);
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
         open_edit_dialog_for_form(stub, this->parentWidget());
   });
   QObject::connect(this->_formActionDuplicate, &QAction::triggered, this, [this]() {
      auto* stub = _get_selected_form(this->ui.table);
      if (!stub)
         return;
      //
      auto& editor = DovahKitCore::get();
      auto result  = editor.duplicate_form(*stub, this);
      if (result)
         this->ui.table->select(result);
   });
   QObject::connect(this->_formActionShowUseInfo, &QAction::triggered, this, [this]() {
      auto* stub = _get_selected_form(this->ui.table);
      if (stub)
         open_use_info_dialog_for_form(stub, this->parentWidget());
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
      auto  request = editor.request_form_renumber(*stub, id);
      if (request.get_error_code() != dovah::default_notice_code) {
         _report_form_renumber_error(this, request.get_error_code());
         return;
      }
      if (!request.commit()) {
         _report_form_renumber_error(this, request.get_error_code());
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
      auto  form_types = this->ui.tree->selectedFormTypes();
      this->_actionCreateForm->setEnabled(form_types.size() == 1);
      this->_formActionEdit->setVisible(stub != nullptr);
      this->_formActionDuplicate->setVisible(stub != nullptr);
      this->_formActionShowUseInfo->setVisible(stub != nullptr);
      this->_formActionRenumber->setVisible(stub != nullptr);
      this->_formActionRenumber->setEnabled(DovahKitCore::get().is_form_defined_in_active_file(stub));
      this->_formActionDelete->setVisible(stub != nullptr);
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