#include "./CameraPathsDialog.h"
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include "dovah/exceptions/form_creation_failed.h"
#include "dovah/forms/CameraPath.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/open_window_for_form.h"
#include "ui/utils/set_custom_context_menu.h"
#include "./CameraPathFormsModel.h"

CameraPathsDialog::CameraPathsDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);
   
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, &QDialog::accept);

   {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         this->_current_form = {};
      });
   }

   {
      QTreeView* widget = this->ui.tree;
      auto*      model  = new CameraPathFormsModel(widget);
      widget->setModel(model);
      this->_model = model;

      {
         auto* breadcrumbs = this->ui.breadcrumbs;
         auto* sel_model   = widget->selectionModel();
         breadcrumbs->setModel(model);
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, breadcrumbs]() {
            const auto qmi     = this->_get_selected_row();
            const auto blocker = QSignalBlocker(breadcrumbs);
            breadcrumbs->setCurrentIndex(qmi);
         });
         QObject::connect(breadcrumbs, &DKBreadcrumbBar::currentIndexChanged, this, [this, sel_model](const QModelIndex& qmi) {
            sel_model->select({ qmi, qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
         });
      }

      widget->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
      widget->setDragDropOverwriteMode(false);
      widget->setDragEnabled(true);
      widget->setAcceptDrops(true);
      widget->setDropIndicatorShown(true);

      widget->installEventFilter(this); // "Del" key deletes a camera path

      // edge-case: moving the currently selected row
      QObject::connect(model, &QAbstractItemModel::rowsMoved, this, [this]() {
         this->_update_move_button_enable_states();
      });
   }

   #pragma region Context menu
   {
      QTreeView* widget = this->ui.tree;
      auto*      menu   = this->_context.menu = new QMenu(widget);
      ui::set_custom_context_menu(*widget, *menu);
      #pragma region Actions
         auto& actions = this->_context.actions;
         {
            auto* action = actions.create_sibling = new QAction(tr("Create sibling.."), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, this, &CameraPathsDialog::_context_create_sibling);
         }
         {
            auto* action = actions.create_child = new QAction(tr("Create child..."), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, this, &CameraPathsDialog::_context_create_child);
         }
         {
            auto* submenu = actions.duplicate = new QMenu(tr("Duplicate camera path"), menu);
            menu->addMenu(submenu);
            {
               auto* action = new QAction(tr("Just this idle"), menu);
               submenu->addAction(action);
               QObject::connect(action, &QAction::triggered, this, &CameraPathsDialog::_context_duplicate_single);
            }
            {
               auto* action = new QAction(tr("This idle and its descendants"), menu);
               submenu->addAction(action);
               QObject::connect(action, &QAction::triggered, this, &CameraPathsDialog::_context_duplicate_tree);
            }
         }
         {
            auto* action = actions.del = new QAction(tr("Delete"), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, this, &CameraPathsDialog::_context_delete_idle);
         }
         {
            auto* action = actions.use_info = new QAction(tr("Use Info"), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, this, &CameraPathsDialog::_context_use_info);
         }
      #pragma endregion

      QObject::connect(menu, &QMenu::aboutToShow, this, [this]() {
         QTreeView* widget    = this->ui.tree;
         auto*      model     = this->_model;
         auto*      sel_model = widget->selectionModel();

         auto qmi     = _get_selected_row();
         bool is_form = model->data(qmi, CameraPathFormsModel::FormStubRole).value<dovah::form_stub*>() != nullptr;

         this->_context.actions.create_sibling->setVisible(is_form);
         this->_context.actions.del->setVisible(is_form);
         this->_context.actions.duplicate->menuAction()->setVisible(is_form);
         this->_context.actions.use_info->setVisible(is_form);
      });
   }
   #pragma endregion

   QObject::connect(this->ui.buttonMoveUp, &QPushButton::clicked, this, [this]() {
      auto* model    = this->_model;
      auto  idle_qmi = this->_get_selected_row();
      model->moveUp(idle_qmi);
   });
   QObject::connect(this->ui.buttonMoveDown, &QPushButton::clicked, this, [this]() {
      auto* model    = this->_model;
      auto  idle_qmi = this->_get_selected_row();
      model->moveDown(idle_qmi);
   });

   this->_set_form_ui_enable_state(false);
   this->_update_move_button_enable_states();

   {  // Push the selected idle to the UI
      auto* sel_model = this->ui.tree->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this]() {
         this->_update_move_button_enable_states();
         this->_pull_selected_idle_to_ui();
      });
   }
   {  // Push changes from the UI to the form instantly upon their occurring
      QObject::connect(this->ui.editorID, &QLineEdit::textEdited, this, &CameraPathsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.mustHaveCameraShots, &QCheckBox::toggled, this, &CameraPathsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.zoom, qOverload<int>(&QComboBox::currentIndexChanged), this, &CameraPathsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.cameraShots, &DKFormListPane::formsAdded,   this, &CameraPathsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.cameraShots, &DKFormListPane::formsRemoved, this, &CameraPathsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.conditions,  &DKConditionList::changed,     this, &CameraPathsDialog::_push_selected_idle_to_form);
   }

   #pragma region Form data editing UI
      {
         auto* widget = this->ui.zoom;
         widget->clear();
         widget->addItem(tr("Default"),   (int)loaded_form_type::zoom_type::default_);
         widget->addItem(tr("Disable"),   (int)loaded_form_type::zoom_type::disable);
         widget->addItem(tr("Shot List"), (int)loaded_form_type::zoom_type::shot_list);
      }
      {
         auto* widget = this->ui.cameraShots;
         widget->setAllowedFormTypes({ dovah::form_type::camera_shot });
      }
   #pragma endregion
}

void CameraPathsDialog::focusCameraPath(dovah::form_stub& stub) {
   if (stub.form_type != dovah::form_type::camera_path)
      return;
   
   auto* widget    = this->ui.tree;
   auto* model     = this->_model;
   auto* sel_model = widget->selectionModel();

   auto qmi = model->index(stub);
   if (!qmi.isValid())
      return;
   widget->scrollTo(qmi); // also expands the treeview as necessary
   sel_model->select({ qmi, qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
}

/*virtual*/ bool CameraPathsDialog::eventFilter(QObject* watched, QEvent* event) /*override*/ {
   if (watched != this->ui.tree)
      return QObject::eventFilter(watched, event);
   if (event->type() == QEvent::KeyPress) {
      auto* casted = (QKeyEvent*)event;
      if (casted->key() == Qt::Key_Delete) {
         this->_keybind_delete_idle();
         return true;
      }
   }
   return false;
}

QModelIndex CameraPathsDialog::_get_selected_row() {
   auto rows = this->ui.tree->selectionModel()->selectedRows();
   if (rows.empty())
      return {};
   return rows[0];
}

#pragma region Idle tree context menu
   void CameraPathsDialog::_context_create_sibling() {
      auto* widget    = this->ui.tree;
      auto* model     = this->_model;
      auto* sel_model = widget->selectionModel();

      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;

      bool ok;
      auto editor_id = QInputDialog::getText(widget, tr("New camera path's editor ID"), tr("Enter an editor ID for the new camera path."), {}, {}, &ok);
      if (!ok || editor_id.isEmpty())
         return;

      QModelIndex idle_qmi;
      try {
         idle_qmi = model->createCameraPathAfter(qmi, editor_id);
      } catch (const dovah::exceptions::form_creation_failed& ex) {
         this->_report_idle_create_error(ex);
         return;
      }
      if (idle_qmi.isValid()) {
         widget->scrollTo(idle_qmi); // also expands the treeview as necessary
         auto tl = idle_qmi.siblingAtColumn(0);
         auto br = idle_qmi.siblingAtColumn(model->columnCount(qmi) - 1);
         sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect); // select new idle
      }
   }
   void CameraPathsDialog::_context_create_child() {
      auto* widget    = this->ui.tree;
      auto* model     = this->_model;
      auto* sel_model = widget->selectionModel();

      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;

      bool ok;
      auto editor_id = QInputDialog::getText(widget, tr("New camera path's editor ID"), tr("Enter an editor ID for the new camera path."), {}, {}, &ok);
      if (!ok || editor_id.isEmpty())
         return;

      QModelIndex idle_qmi;
      try {
         idle_qmi = model->createCameraPath(qmi, editor_id);
      } catch (const dovah::exceptions::form_creation_failed& ex) {
         this->_report_idle_create_error(ex);
         return;
      }
      if (idle_qmi.isValid()) {
         widget->scrollTo(idle_qmi); // also expands the treeview as necessary
         auto tl = idle_qmi.siblingAtColumn(0);
         auto br = idle_qmi.siblingAtColumn(model->columnCount(qmi) - 1);
         sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect); // select new idle
      }
   }
   void CameraPathsDialog::_context_duplicate_single() {
      auto* widget    = this->ui.tree;
      auto* model     = this->_model;
      auto* sel_model = widget->selectionModel();

      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;

      QModelIndex idle_qmi;
      try {
         idle_qmi = model->duplicateCameraPath(qmi, false);
      } catch (const CameraPathFormsModel::too_many_to_duplicate_exception& ex) {
         QMessageBox::critical(
            this,
            QObject::tr("Error", "create new form error"),
            QObject::tr("Unable to duplicate this idle animation. The active file contains too many forms already.")
         );
         return;
      } catch (const dovah::exceptions::form_creation_failed& ex) {
         this->_report_idle_create_error(ex);
         return;
      }
      if (!idle_qmi.isValid())
         return;

      widget->scrollTo(idle_qmi); // also expands the treeview as necessary
      auto tl = idle_qmi.siblingAtColumn(0);
      auto br = idle_qmi.siblingAtColumn(model->columnCount({}) - 1);
      sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect); // select new idle
   }
   void CameraPathsDialog::_context_duplicate_tree() {
      auto* widget    = this->ui.tree;
      auto* model     = this->_model;
      auto* sel_model = widget->selectionModel();

      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;
   
      QModelIndex idle_qmi;
      try {
         idle_qmi = model->duplicateCameraPath(qmi, true);
      } catch (const CameraPathFormsModel::too_many_to_duplicate_exception& ex) {
         QMessageBox::critical(
            this,
            QObject::tr("Error", "create new form error"),
            QObject::tr(
               "Unable to duplicate this idle animation. The active file contains too many forms already; "
               "of the %1 form IDs needed for the new idles, only %2 are available"
            )
               .arg(ex.form_id_counts.needed)
               .arg(ex.form_id_counts.available)
         );
         return;
      } catch (const dovah::exceptions::form_creation_failed& ex) {
         this->_report_idle_create_error(ex);
         return;
      }
      if (!idle_qmi.isValid())
         return;

      widget->scrollTo(idle_qmi); // also expands the treeview as necessary
      auto tl = idle_qmi.siblingAtColumn(0);
      auto br = idle_qmi.siblingAtColumn(model->columnCount({}) - 1);
      sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect); // select new idle
   }
   void CameraPathsDialog::_context_delete_idle() {
      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;
      auto* stub = this->_model->data(qmi, CameraPathFormsModel::FormStubRole).value<dovah::form_stub*>();
      if (!stub)
         return;

      if (this->_current_form) {
         if (stub == &this->_current_form->stub) {
            this->_current_form = nullptr;
         }
      }
      this->_model->deleteCameraPath(qmi, this);
   }
   void CameraPathsDialog::_context_use_info() {
      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;
      auto* stub = this->_model->data(qmi, CameraPathFormsModel::FormStubRole).value<dovah::form_stub*>();
      if (!stub)
         return;

      open_use_info_dialog_for_form(*stub);
   }
#pragma endregion

void CameraPathsDialog::_keybind_delete_idle() {
   auto* model  = this->_model;

   auto  qmi  = _get_selected_row();
   auto* stub = model->data(qmi, CameraPathFormsModel::FormStubRole).value<dovah::form_stub*>();
   if (!stub) {
      QApplication::beep();
      return;
   }
   model->deleteCameraPath(qmi, this);
}

void CameraPathsDialog::_report_idle_create_error(const dovah::exceptions::form_creation_failed& ex) {
   QString text;
   switch (ex.code) {
      using enum dovah::exceptions::form_creation_failed::error_code;
      case invalid_form_type:
         text = QObject::tr("An internal program error occurred: DovahKit tried to create a form but supplied a bad form type. (Wait, what? How did you get the Idle Animations Window to try to do that?)");
         break;
      case no_active_file:
         text = QObject::tr("There is no active file, nor any room in the load order for a new file.");
         break;
      case no_form_id_available:
         text = QObject::tr("You've used up all of the form IDs available to this file!");
         break;
      case unimplemented_form_type:
         text = QObject::tr("DovahKit does not support editing this form type. (Wait, what? How did you get the Idle Animations Window to try to do that?)");
         break;
      case invalid_parent_child_relationship:
         text = QObject::tr("The specified parent form cannot have a child form of this type. (Wait, what? How did you get the Idle Animations Window to try to do that?)");
         break;
      case exterior_grid_coordinates_already_taken:
         text = QObject::tr("The specified worldspace already has an exterior cell at the desired grid coordinates. (Wait, what? How did you get the Idle Animations Window to try and create an exterior cell?)");
         break;
      case cannot_create_reference_with_no_parent_cell:
         text = QObject::tr("References cannot be created outside of a cell. (Wait, what? How did you get the Idle Animations Window to try and create a reference?)");
         break;
      case interior_cell_clone_cannot_have_parent:
         text = QObject::tr("Interior cells cannot have a parent worldspace. (Wait, what? How did you get the Idle Animations Window to try and create an interior cell?)");
         break;
      case exterior_cell_clone_must_have_parent:
         text = QObject::tr("Exterior cells must have a parent worldspace. (Wait, what? How did you get the Idle Animations Window to try and create an exterior cell?)");
         break;
      case cannot_sever_references_to_none_stub:
         text = QObject::tr("DovahKit needed to select a form ID to use for the new form. The chosen form ID is the target of one or more dangling references, and DovahKit does not know how to sever those references, so the form creation process could not continue.");
         break;
      case form_type_unavailable_in_current_game:
         text = QObject::tr("The desired form type doesn't exist in the version (e.g. Classic/Special) of Skyrim this file was created for. Try converting the file to the target Skyrim version first. (Wait, what? How did you get the Idle Animations Window to try to do that?)");
         break;
   }
   QMessageBox::critical(
      this,
      QObject::tr("Error", "create new form error"),
      QObject::tr("Unable to create a new idle animation. %1").arg(text)
   );
}

void CameraPathsDialog::_update_move_button_enable_states() {
   auto* model    = this->_model;
   auto  idle_qmi = this->_get_selected_row();
   this->ui.buttonMoveUp->setEnabled(model->canMoveUp(idle_qmi));
   this->ui.buttonMoveDown->setEnabled(model->canMoveDown(idle_qmi));
}

void CameraPathsDialog::_pull_selected_idle_to_ui() {
   dovah::form_stub* stub = this->_model->data(_get_selected_row(), CameraPathFormsModel::FormStubRole).value<dovah::form_stub*>();
   if (!stub) {
      this->_set_form_ui_enable_state(this->_current_form != nullptr);
      return;
   }

   this->_current_form = stub->load().ptr_cast<loaded_form_type>();
   if (!this->_current_form) {
      this->_set_form_ui_enable_state(false);
      return;
   }
   auto& loaded = *this->_current_form;

   const auto blockers = std::array{
      QSignalBlocker(this->ui.editorID),
      QSignalBlocker(this->ui.mustHaveCameraShots),
      QSignalBlocker(this->ui.zoom),
      QSignalBlocker(this->ui.cameraShots),
      QSignalBlocker(this->ui.conditions),
   };

   this->ui.editorID->setText(stub->get_editor_id());
   {
      auto* widget = this->ui.zoom;
      auto  i      = widget->findData((int)loaded.zoom.type);
      if (i < 0)
         i = 0;
      widget->setCurrentIndex(i);
   }
   this->ui.mustHaveCameraShots->setChecked(loaded.zoom.must_have_camera_shots);
   this->ui.cameraShots->pullStubs(loaded.camera_shots);
   this->ui.conditions->importFrom(loaded, loaded.conditions);

   this->_set_form_ui_enable_state(true);
}
void CameraPathsDialog::_push_selected_idle_to_form() {
   if (!this->_current_form)
      return;

   auto& loaded = *this->_current_form;
   auto& editor = DovahKitCore::get();
   emit editor.formModificationImminent(&loaded.stub);

   loaded.stub.editorID = this->ui.editorID->text().toStdString();
   loaded.zoom.type     = (loaded_form_type::zoom_type)this->ui.zoom->currentData().toInt();
   loaded.zoom.must_have_camera_shots = this->ui.mustHaveCameraShots->isChecked();
   this->ui.cameraShots->commitStubs(loaded.camera_shots, loaded);
   this->ui.conditions->exportTo(loaded, loaded.conditions);

   loaded.stub.set_edited(true);
   emit editor.formModified(&loaded.stub);
}
void CameraPathsDialog::_set_form_ui_enable_state(bool v) {
   const auto widgets = std::array<QWidget*, 5>{
      this->ui.editorID,
      this->ui.mustHaveCameraShots,
      this->ui.zoom,
      this->ui.cameraShots,
      this->ui.conditions,
   };
   for (auto* widget : widgets)
      widget->setEnabled(v);
}