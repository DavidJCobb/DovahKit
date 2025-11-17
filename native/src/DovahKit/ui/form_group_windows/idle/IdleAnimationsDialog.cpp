#include "./IdleAnimationsDialog.h"
#include <QInputDialog>
#include <QMessageBox>
#include "dovah/exceptions/form_creation_failed.h"
#include "dovah/forms/IdleAnimation.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "ui/utils/set_custom_context_menu.h"
#include "./IdleAnimationFormsModel.h"
#include "./FormSubdialogIdleNewActionRoot.h"
#include "./FormSubdialogIdleNewGraph.h"

IdleAnimationsDialog::IdleAnimationsDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);
   
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, &QDialog::accept);

   {
      auto* widget = this->ui.idles;
      auto* model  = new IdleAnimationFormsModel(widget);
      widget->setModel(model);
   }

   #pragma region Context menu
   {
      auto* widget = this->ui.idles;
      auto* menu = this->_context.menu = new QMenu(widget);
      ui::set_custom_context_menu(*widget, *menu);
      //
      #pragma region Graph actions
      {
         auto& group = this->_context.actions.graph;
         {
            auto* action = group.create = new QAction(tr("Create behavior graph..."), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, this, &IdleAnimationsDialog::_context_add_graph);
         }
         {
            auto* action = group.add_action_root = new QAction(tr("Add action..."), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, this, &IdleAnimationsDialog::_context_add_action_root);
         }
      }
      #pragma endregion
      #pragma region Idle actions
      {
         auto& group = this->_context.actions.idle;
         {
            auto* action = group.create = new QAction(tr("Create child idle..."), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, this, &IdleAnimationsDialog::_context_add_idle);
         }
         {
            auto* submenu = group.duplicate = new QMenu(tr("Duplicate idle"), menu);
            menu->addMenu(submenu);
            {
               auto* action = new QAction(tr("Just this idle"), menu);
               submenu->addAction(action);
               QObject::connect(action, &QAction::triggered, this, &IdleAnimationsDialog::_context_duplicate_idle_single);
            }
            {
               auto* action = new QAction(tr("This idle and its descendants"), menu);
               submenu->addAction(action);
               QObject::connect(action, &QAction::triggered, this, &IdleAnimationsDialog::_context_duplicate_idle_tree);
            }
         }
         {
            auto* action = group.del = new QAction(tr("Delete idle"), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, this, &IdleAnimationsDialog::_context_delete_idle);
         }
      }
      #pragma endregion

      QObject::connect(menu, &QMenu::aboutToShow, this, [this]() {
         auto* widget    = this->ui.idles;
         auto* model     = (IdleAnimationFormsModel*) widget->model();
         auto* sel_model = widget->selectionModel();

         auto qmi = _get_selected_row();

         bool is_none        = !qmi.isValid();
         bool is_graph       = false;
         bool is_action      = false;
         bool is_idle        = false;
         bool is_idle_parent = false;
         if (!is_none) {
            using enum IdleAnimationFormsModel::NodeType;
            //
            const auto node_type = model->data(qmi, IdleAnimationFormsModel::NodeTypeRole).value<IdleAnimationFormsModel::NodeType>();
            is_graph       = node_type == Graph;
            is_action      = node_type == Action;
            is_idle        = node_type == Idle;
            is_idle_parent = is_action || is_idle || node_type == LooseIdlesPerGraph || node_type == LooseIdlesPerModel;
         }
         bool can_duplicate_idle = false;
         bool can_delete_idle    = false;
         if (is_idle) {
            can_duplicate_idle = model->canEverDuplicateIdle(qmi);
            can_delete_idle    = model->canDeleteIdle(qmi);
         }

         this->_context.actions.graph.create->setVisible(is_none || is_graph);
         this->_context.actions.graph.add_action_root->setVisible(is_graph);

         this->_context.actions.idle.create->setVisible(is_idle_parent);
         this->_context.actions.idle.duplicate->menuAction()->setVisible(can_duplicate_idle);
         this->_context.actions.idle.del->setVisible(can_delete_idle);
      });
   }
   #pragma endregion

   //
   // TODO: Drag-and-drop (most of the implementation will need to be in the model)
   //

   //
   // TODO: "Move Up" and "Move Down" buttons for idles that are children of another idle
   //
   
   {  // Push the selected idle to the UI
      auto* sel_model = this->ui.idles->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this]() {
         this->_pull_selected_idle_to_ui();
      });
   }
   {  // Push changes from the UI to the form instantly upon their occurring
      QObject::connect(this->ui.editorID,        &QLineEdit::textEdited, this, &IdleAnimationsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.flagBlocking,    &QCheckBox::toggled,    this, &IdleAnimationsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.flagNoAttacking, &QCheckBox::toggled,    this, &IdleAnimationsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.flagSequence,    &QCheckBox::toggled,    this, &IdleAnimationsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.animEvent,       &QLineEdit::textEdited, this, &IdleAnimationsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.loopForever,     &QRadioButton::toggled, this, &IdleAnimationsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.loopLimited,     &QRadioButton::toggled, this, &IdleAnimationsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.loopTimeMin,     qOverload<int>(&QSpinBox::valueChanged), this, &IdleAnimationsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.loopTimeMax,     qOverload<int>(&QSpinBox::valueChanged), this, &IdleAnimationsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.repeatDelay,     qOverload<int>(&QSpinBox::valueChanged), this, &IdleAnimationsDialog::_push_selected_idle_to_form);
      QObject::connect(this->ui.conditions,      &DKConditionList::changed, this, &IdleAnimationsDialog::_push_selected_idle_to_form);
   }
}

void IdleAnimationsDialog::focusIdle(dovah::form_stub& idle) {
   if (idle.form_type != dovah::form_type::idle)
      return;
   
   auto* widget    = this->ui.idles;
   auto* model     = (IdleAnimationFormsModel*) widget->model();
   auto* sel_model = widget->selectionModel();

   auto qmi = model->idleQMI(idle);
   if (!qmi.isValid())
      return;
   widget->scrollTo(qmi); // also expands the treeview as necessary
   sel_model->select({ qmi, qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
}

QModelIndex IdleAnimationsDialog::_get_selected_row() {
   auto rows = this->ui.idles->selectionModel()->selectedRows();
   if (rows.empty())
      return {};
   return rows[0];
}

#pragma region Idle tree context menu
   void IdleAnimationsDialog::_context_add_action_root() {
      auto* widget    = this->ui.idles;
      auto* model     = (IdleAnimationFormsModel*) widget->model();
      auto* sel_model = widget->selectionModel();

      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;
      auto node_type = model->data(qmi, IdleAnimationFormsModel::NodeTypeRole).value<IdleAnimationFormsModel::NodeType>();
      if (node_type != IdleAnimationFormsModel::NodeType::Graph)
         return;

      dovah::form_stub* actionStub = nullptr;
      QString idleEditorID;
      {
         FormSubdialogIdleNewActionRoot modal(this);
         modal.setModal(true);
         modal.setExistingActionRoots(model->actionsByGraph(qmi));
         if (modal.exec() == QDialog::Rejected)
            return;

         actionStub = modal.action();
         if (!actionStub)
            return;
         idleEditorID = modal.idleEditorID();
      }

      QModelIndex idle_qmi;
      try {
         idle_qmi = model->createActionRoot(qmi, *actionStub, idleEditorID);
      } catch (const dovah::exceptions::form_creation_failed& ex) {
         this->_report_idle_create_error(ex);
         return;
      }
      if (idle_qmi.isValid()) {
         auto parent_qmi = model->parent(idle_qmi);
         widget->scrollTo(idle_qmi); // also expands the treeview as necessary
         auto tl = idle_qmi.siblingAtColumn(0);
         auto br = idle_qmi.siblingAtColumn(model->columnCount(parent_qmi) - 1);
         sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect); // select new idle
      }
   }
   void IdleAnimationsDialog::_context_add_graph() {
      QString path;
      {
         FormSubdialogIdleNewGraph modal(this);
         modal.setModal(true);
         if (modal.exec() == QDialog::Rejected)
            return;

         path = modal.path();
         if (path.isEmpty())
            return;
      }

      auto* widget    = this->ui.idles;
      auto* model     = (IdleAnimationFormsModel*) widget->model();
      auto* sel_model = widget->selectionModel();

      QModelIndex qmi = model->getOrCreateGraph(path);
      if (qmi.isValid()) {
         auto parent_qmi = model->parent(qmi);
         widget->scrollTo(qmi); // also expands the treeview as necessary
         auto tl = qmi.siblingAtColumn(0);
         auto br = qmi.siblingAtColumn(model->columnCount(parent_qmi) - 1);
         sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect); // select new idle
      }
   }
   void IdleAnimationsDialog::_context_add_idle() {
      auto* widget    = this->ui.idles;
      auto* model     = (IdleAnimationFormsModel*)widget->model();
      auto* sel_model = widget->selectionModel();

      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;
      if (!model->canCreateIdleIn(qmi))
         return;

      bool ok;
      auto editor_id = QInputDialog::getText(widget, tr("New idle's editor ID"), tr("Enter an editor ID for the new idle."), {}, {}, &ok);
      if (!ok || editor_id.isEmpty())
         return;

      QModelIndex idle_qmi;
      try {
         idle_qmi = model->createIdle(qmi, editor_id);
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
   void IdleAnimationsDialog::_context_duplicate_idle_single() {
      auto* widget    = this->ui.idles;
      auto* model     = (IdleAnimationFormsModel*)widget->model();
      auto* sel_model = widget->selectionModel();

      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;
      if (!model->canEverDuplicateIdle(qmi))
         return;

      QModelIndex idle_qmi;
      try {
         idle_qmi = model->duplicateIdle(qmi, false);
      } catch (const IdleAnimationFormsModel::too_many_to_duplicate_exception& ex) {
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
   void IdleAnimationsDialog::_context_duplicate_idle_tree() {
      auto* widget    = this->ui.idles;
      auto* model     = (IdleAnimationFormsModel*)widget->model();
      auto* sel_model = widget->selectionModel();

      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;
      if (!model->canEverDuplicateIdle(qmi))
         return;
   
      QModelIndex idle_qmi;
      try {
         idle_qmi = model->duplicateIdle(qmi, true);
      } catch (const IdleAnimationFormsModel::too_many_to_duplicate_exception& ex) {
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
   void IdleAnimationsDialog::_context_delete_idle() {
      auto* widget    = this->ui.idles;
      auto* model     = (IdleAnimationFormsModel*)widget->model();
      auto* sel_model = widget->selectionModel();

      auto qmi = _get_selected_row();
      if (!qmi.isValid())
         return;
      if (!model->canDeleteIdle(qmi))
         return;

      auto* stub = model->data(qmi, IdleAnimationFormsModel::FormStubRole).value<dovah::form_stub*>();
      if (!stub)
         return;

      model->deleteIdle(qmi, this);
   }
#pragma endregion

void IdleAnimationsDialog::_report_idle_create_error(const dovah::exceptions::form_creation_failed& ex) {
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

void IdleAnimationsDialog::_pull_selected_idle_to_ui() {
   dovah::form_stub* stub = nullptr;
   {
      auto* widget = this->ui.idles;
      auto* model  = (IdleAnimationFormsModel*)widget->model();
      stub = model->data(_get_selected_row(), IdleAnimationFormsModel::FormStubRole).value<dovah::form_stub*>();
   }
   if (!stub) {
      this->_set_form_ui_enable_state(this->_current_idle != nullptr);
      return;
   }

   this->_current_idle = stub->load().ptr_cast<loaded_form_type>();
   if (!this->_current_idle) {
      this->_set_form_ui_enable_state(false);
      return;
   }
   auto& loaded = *this->_current_idle;

   const auto blockers = std::array{
      QSignalBlocker(this->ui.editorID),
      QSignalBlocker(this->ui.flagBlocking),
      QSignalBlocker(this->ui.flagNoAttacking),
      QSignalBlocker(this->ui.flagSequence),
      QSignalBlocker(this->ui.animEvent),
      QSignalBlocker(this->ui.loopForever),
      QSignalBlocker(this->ui.loopLimited),
      QSignalBlocker(this->ui.loopTimeMin),
      QSignalBlocker(this->ui.loopTimeMax),
      QSignalBlocker(this->ui.repeatDelay),
      QSignalBlocker(this->ui.conditions),
   };

   this->ui.editorID->setText(stub->get_editor_id());

   this->ui.flagBlocking->setChecked(loaded.data.flags & loaded_form_type::flag::blocking);
   this->ui.flagNoAttacking->setChecked(loaded.data.flags & loaded_form_type::flag::no_attacking);
   this->ui.flagSequence->setChecked(loaded.data.flags & loaded_form_type::flag::sequence);

   this->ui.animEvent->setText(QString::fromStdString(loaded.animation_event));

   if (loaded.loops_forever()) {
      this->ui.loopForever->setChecked(true);
      this->ui.loopLimited->setChecked(false);
      this->ui.loopTimeMin->setEnabled(false);
      this->ui.loopTimeMax->setEnabled(false);
   } else {
      this->ui.loopForever->setChecked(false);
      this->ui.loopLimited->setChecked(true);
      this->ui.loopTimeMin->setEnabled(true);
      this->ui.loopTimeMax->setEnabled(true);
      this->ui.loopTimeMin->setValue(loaded.data.loop_time_range.min);
      this->ui.loopTimeMax->setValue(loaded.data.loop_time_range.max);
   }
   this->ui.repeatDelay->setValue(loaded.data.replay_delay);

   this->ui.conditions->importFrom(loaded, loaded.conditions);

   this->_set_form_ui_enable_state(true);
}
void IdleAnimationsDialog::_push_selected_idle_to_form() {
   if (!this->_current_idle)
      return;

   auto& loaded = *this->_current_idle;
   auto& editor = DovahKitCore::get();
   emit editor.formModificationImminent(&loaded.stub);

   loaded.stub.editorID = this->ui.editorID->text().toStdString();
   {
      auto& flags = loaded.data.flags;
      cobb::edit_bit(flags, loaded_form_type::flag::blocking,     this->ui.flagBlocking->isChecked());
      cobb::edit_bit(flags, loaded_form_type::flag::no_attacking, this->ui.flagNoAttacking->isChecked());
      cobb::edit_bit(flags, loaded_form_type::flag::sequence,     this->ui.flagSequence->isChecked());
   }
   if (this->ui.loopForever->isChecked()) {
      loaded.data.loop_time_range.min = loaded_form_type::loop_time_forever;
      loaded.data.loop_time_range.max = loaded_form_type::loop_time_forever;
   } else {
      loaded.data.loop_time_range.min = this->ui.loopTimeMin->value();
      loaded.data.loop_time_range.max = this->ui.loopTimeMax->value();
   }
   loaded.data.replay_delay = this->ui.repeatDelay->value();
   this->ui.conditions->exportTo(loaded, loaded.conditions);

   emit editor.formModified(&loaded.stub);
}
void IdleAnimationsDialog::_set_form_ui_enable_state(bool v) {
   const auto widgets = std::array<QWidget*, 11>{
      this->ui.editorID,
      this->ui.flagBlocking,
      this->ui.flagNoAttacking,
      this->ui.flagSequence,
      this->ui.animEvent,
      this->ui.loopForever,
      this->ui.loopLimited,
      this->ui.loopTimeMin,
      this->ui.loopTimeMax,
      this->ui.repeatDelay,
      this->ui.conditions,
   };
   for (auto* widget : widgets)
      widget->setEnabled(v);
}