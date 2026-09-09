#include "./object_window.h"
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include "dovah/exceptions/form_creation_failed.h"
#include "dovah/exceptions/form_renumber_failed.h"
#include "dovah/forms/Quest.h"
#include "dovah/utils/form_type_is_object_with_bounds.h"
#include "editor/core.h"
#include "editor/open_window_for_form.h"
#include "editor/helpers/is_form_type_legal_to_create.h"
#include "editor/helpers/make_editor_id_for_duplicate.h"
#include "editor/helpers/recalc_bounds.h"
#include "editor/localize/form_creation_error_code.h"
#include "./form_use_info.h"

namespace {
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
   
   this->ui.table->setSource(this->ui.tree);
   this->ui.table->setFilter(this->ui.filter);
   this->ui.table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
   QObject::connect(this->ui.table, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      auto* stub = _get_selected_form();
      if (stub && !stub->is_none_stub())
         open_edit_dialog_for_form(*stub, this);
   });
  
   #pragma region Form table context menu
   {
      auto& context = this->context_menus.form_table;
      auto& menu    = context.menu;
      auto& actions = context.actions;
      auto* widget  = this->ui.table;
      {
         actions.create_form   = menu.addAction(tr("New form...",       "object window form actions"), this, &ObjectWindow::_create_form_in_current_category);
         actions.duplicate     = menu.addAction(tr("Duplicate",         "object window form actions"), this, &ObjectWindow::_selected_form_duplicate);
         actions.edit          = menu.addAction(tr("Edit",              "object window form actions"), this, &ObjectWindow::_selected_form_edit);
         actions.show_use_info = menu.addAction(tr("Use info",          "object window form actions"), this, &ObjectWindow::_selected_form_show_users);
         actions.renumber      = menu.addAction(tr("Change form ID...", "object window form actions"), this, &ObjectWindow::_selected_form_renumber);
         actions.delete_form   = menu.addAction(tr("Delete",            "object window form actions"), this, &ObjectWindow::_selected_form_delete);
         actions.separator = menu.addSeparator();
         actions.recalc_bounds = menu.addAction(tr("Recalc bounds", "object window form actions"), this, &ObjectWindow::_selected_form_recalc_bounds);
      }
      widget->setContextMenuPolicy(Qt::CustomContextMenu);
      QObject::connect(widget, &QWidget::customContextMenuRequested, this, [this, widget, &context](const QPoint& pos) {
         auto& actions = context.actions;
         auto& menu    = context.menu;
         
         auto* stub       = _get_selected_form();
         auto  form_types = this->ui.tree->filterInfo().form_types;
         
         bool form_exists  = (stub != nullptr);
         bool is_alterable = form_exists && !stub->is_none_stub();
         
         actions.create_form->setEnabled(form_types.size() == 1 && form_types[0] != dovah::form_type::none);
         actions.duplicate->setEnabled(is_alterable);
         actions.duplicate->setVisible(is_alterable);
         actions.edit->setVisible(is_alterable);
         actions.show_use_info->setVisible(form_exists);
         actions.renumber->setVisible(is_alterable);
         actions.renumber->setEnabled(is_alterable && DovahKitCore::get().is_form_defined_in_active_file(stub));
         actions.recalc_bounds->setVisible(is_alterable && dovah::form_type_is_object_with_bounds(stub->form_type));
         actions.delete_form->setVisible(is_alterable);
         if (form_types.size() == 1) {
            auto type = form_types[0];
            if (!editor_helpers::is_form_type_legal_to_create(type)) {
               actions.create_form->setEnabled(false);
            }
         }
         if (stub) {
            if (!editor_helpers::is_form_type_legal_to_create(stub->form_type)) {
               actions.duplicate->setEnabled(false);
            }
         }

         actions.separator->setVisible(actions.recalc_bounds->isVisible());

         if (menu.isEmpty())
            return; // don't show a menu if all of its contents are disabled or hidden
         menu.exec(widget->mapToGlobal(pos));
      });
   }
   #pragma endregion
   #pragma region Category tree context menu
   {
      auto& context = this->context_menus.tree;
      auto& menu    = context.menu;
      auto& actions = context.actions;
      auto* widget  = this->ui.tree;
      {
         auto* action = actions.collapse_children = menu.addAction(tr("Collapse children"));
         QObject::connect(action, &QAction::triggered, this, [this, widget]() {
            this->_set_selected_category_contents_expanded(false, false);
         });
      }
      {
         auto* action = actions.expand_children = menu.addAction(tr("Expand children"));
         QObject::connect(action, &QAction::triggered, this, [this, widget]() {
            this->_set_selected_category_contents_expanded(true, false);
         });
      }
      {
         auto* action = actions.collapse_descendants = menu.addAction(tr("Collapse all descendants"));
         QObject::connect(action, &QAction::triggered, this, [this, widget]() {
            this->_set_selected_category_contents_expanded(false, true);
         });
      }
      {
         auto* action = actions.expand_descendants = menu.addAction(tr("Expand all descendants"));
         QObject::connect(action, &QAction::triggered, this, [this, widget]() {
            this->_set_selected_category_contents_expanded(true, true);
         });
      }
      widget->setContextMenuPolicy(Qt::CustomContextMenu);
      QObject::connect(widget, &QWidget::customContextMenuRequested, this, [this, widget, &context](const QPoint& pos) {
         auto& actions = context.actions;
         auto& menu    = context.menu;
         auto* model   = widget->model();
         assert(!!model);

         QModelIndex qmi;
         {
            auto* sm = widget->selectionModel();
            assert(!!sm);
            auto rows = sm->selectedRows();
            if (rows.isEmpty())
               return;
            qmi = rows[0];
         }
         if (!qmi.isValid())
            return;

         const int row_count = model->rowCount(qmi);
         if (row_count <= 0)
            return;

         menu.exec(widget->mapToGlobal(pos));
      });
   }
   #pragma endregion

   // have to do after configuring the UI; doing this in the treeview constructor 
   // completely breaks layout unless and until the tree structure changes after 
   // loading content. no clue why. Qt does not give me the means to debug this.
   this->ui.tree->expandAll();
}

dovah::form_stub* ObjectWindow::_get_selected_form() {
   auto* proxy  = (QSortFilterProxyModel*)this->ui.table->model();
   auto  select = this->ui.table->selectionModel()->selection().indexes();
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

void ObjectWindow::_create_form_in_current_category() {
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
   if (created_form) {
      if (created_form->form_type == dovah::form_type::quest) {
         //
         // If we're currently viewing a Quest Filter, give the new quest that filter, so that it isn't 
         // immediately filtered out of view.
         //
         auto filter = this->ui.tree->filterInfo().filters.quest_filter_prefix;
         if (!filter.isEmpty()) {
            auto loaded = created_form->load().ptr_cast<dovah::loaded_forms::Quest>();
            if (loaded) {
               auto& editor = DovahKitCore::get();
               emit editor.formModificationImminent(created_form);
               loaded->filter = filter.toStdString();
               created_form->set_edited(true);
               emit editor.formModified(created_form);
            }
         }
      }
      this->ui.table->select(created_form);
   }
}
void ObjectWindow::_selected_form_edit() {
   if (auto* stub = _get_selected_form())
      open_edit_dialog_for_form(*stub, this->parentWidget());
}
void ObjectWindow::_selected_form_duplicate() {
   auto* stub = _get_selected_form();
   if (!stub)
      return;
   dovah::form_stub* created_form = DovahKitCore::get().duplicate_form(*stub, this);
   if (created_form)
      this->ui.table->select(created_form);
}
void ObjectWindow::_selected_form_show_users() {
   if (auto* stub = _get_selected_form())
      open_use_info_dialog_for_form(*stub, this->parentWidget());
}
void ObjectWindow::_selected_form_recalc_bounds() {
   if (auto* stub = _get_selected_form())
      editor_helpers::recalc_bounds(this, *stub);
}
void ObjectWindow::_selected_form_renumber() {
   auto* stub = _get_selected_form();
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
}
void ObjectWindow::_selected_form_delete() {
   if (auto* stub = _get_selected_form())
      DovahKitCore::get().delete_form(*stub, this);
}

void ObjectWindow::_set_selected_category_contents_expanded(bool expand, bool recurse) {
   auto* widget = this->ui.tree;
   auto* model  = widget->model();
   auto* sm     = widget->selectionModel();
   assert(!!model);
   assert(!!sm);
   QModelIndex qmi;
   {
      auto rows = sm->selectedRows();
      if (rows.empty())
         return;
      qmi = rows[0];
   }
   if (!qmi.isValid())
      return;

   if (expand) {
      auto row_count = model->rowCount(qmi);
      for (int i = 0; i < row_count; ++i) {
         auto child_qmi = model->index(i, 0, qmi);
         widget->expandRecursively(child_qmi, recurse ? -1 : 0);
      }
      return;
   }
   //
   // There's no "collapse recursively" function.
   //
   if (!recurse) {
      const auto row_count = model->rowCount(qmi);
      for (int i = 0; i < row_count; ++i)
         widget->collapse(model->index(i, 0, qmi));
      return;
   }
   auto _collapse_descendants = [widget, model](this auto&& recurse, const QModelIndex& qmi) -> void {
      const auto row_count = model->rowCount(qmi);
      for (int i = 0; i < row_count; ++i) {
         const auto child_qmi = model->index(i, 0, qmi);
         widget->collapse(child_qmi);
         recurse(child_qmi);
      }
   };
   _collapse_descendants(qmi);
}