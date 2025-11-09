#include "./IdleAnimationsDialog.h"
#include <QInputDialog>
#include "dovah/forms/IdleAnimation.h"
#include "dovah/form_stub.h"
#include "ui/utils/set_custom_context_menu.h"

// TODO: Move these to ui/form_group_windows/idle:
#include "ui/form_windows/idle/IdleAnimationFormsModel_2.h" // TODO: delete first model; ditch "_2" prefix on this model
#include "ui/form_windows/idle/FormSubdialogIdleNewActionRoot.h"

IdleAnimationsDialog::IdleAnimationsDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);
   
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, &QDialog::accept);

   {
      auto* widget = this->ui.idles;
      auto* model  = new IdleAnimationFormsModel_2(widget);
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
            auto* action = group.add_action_root = new QAction(tr("Add action"), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, this, &IdleAnimationsDialog::_context_add_action_root);
         }
         //
         // TODO: New graph
         //
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
         //
         // TODO: "Delete idle" action
         //
      }
      #pragma endregion

      //
      // TODO: Show/hide actions based on what was right-clicked
      //
   }
   #pragma endregion

   //
   // TODO: Drag-and-drop (most of the implementation will need to be in the model)
   //

   //
   // TODO: "Move Up" and "Move Down" buttons for idles that are children of another idle
   //
   // TODO: Selecting an idle to view its fields
   // 
   // TODO: Editing fields of a selected idle should push changes to the form immediately
   //
}

QModelIndex IdleAnimationsDialog::_get_selected_row() {
   auto rows = this->ui.idles->selectionModel()->selectedRows();
   if (rows.empty())
      return {};
   return rows[0];
}

void IdleAnimationsDialog::_context_add_action_root() {
   auto* widget    = this->ui.idles;
   auto* model     = (IdleAnimationFormsModel_2*) widget->model();
   auto* sel_model = widget->selectionModel();

   auto qmi = _get_selected_row();
   if (!qmi.isValid())
      return;
   auto node_type = model->data(qmi, IdleAnimationFormsModel_2::NodeTypeRole).value<IdleAnimationFormsModel_2::NodeType>();
   if (node_type != IdleAnimationFormsModel_2::NodeType::Graph)
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

   // TODO: This function should be allowed to throw form-creation exceptions, so we can 
   // catch them here and show error messages to the user. Currently, it just swallows 
   // exceptions and returns an intentionally invalid QMI.
   auto idle_qmi = model->createActionRoot(qmi, *actionStub, idleEditorID);
   if (idle_qmi.isValid()) {
      auto parent_qmi = model->parent(idle_qmi);
      widget->scrollTo(idle_qmi); // also expands the treeview as necessary
      auto tl = idle_qmi.siblingAtColumn(0);
      auto br = idle_qmi.siblingAtColumn(model->columnCount(parent_qmi) - 1);
      sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect); // select new idle
   }
}
void IdleAnimationsDialog::_context_add_idle() {
   auto* widget    = this->ui.idles;
   auto* model     = (IdleAnimationFormsModel_2*)widget->model();
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

   // TODO: This function should be allowed to throw form-creation exceptions, so we can 
   // catch them here and show error messages to the user. Currently, it just swallows 
   // exceptions and returns an intentionally invalid QMI.
   auto idle_qmi = model->createIdle(qmi, editor_id);
   if (idle_qmi.isValid()) {
      widget->scrollTo(idle_qmi); // also expands the treeview as necessary
      auto tl = idle_qmi.siblingAtColumn(0);
      auto br = idle_qmi.siblingAtColumn(model->columnCount(qmi) - 1);
      sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect); // select new idle
   }
}
void IdleAnimationsDialog::_context_duplicate_idle_single() {
   auto* widget    = this->ui.idles;
   auto* model     = (IdleAnimationFormsModel_2*)widget->model();
   auto* sel_model = widget->selectionModel();

   auto qmi = _get_selected_row();
   if (!qmi.isValid())
      return;
   if (!model->canEverDuplicateIdle(qmi))
      return;

   // TODO: This function should be allowed to throw form-creation exceptions, so we can 
   // catch them here and show error messages to the user. Currently, it just swallows 
   // exceptions and returns an intentionally invalid QMI.
   auto idle_qmi = model->duplicateIdle(qmi, false);
   if (!idle_qmi.isValid()) {
      return;
   }

   widget->scrollTo(idle_qmi); // also expands the treeview as necessary
   auto tl = idle_qmi.siblingAtColumn(0);
   auto br = idle_qmi.siblingAtColumn(model->columnCount({}) - 1);
   sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect); // select new idle
}
void IdleAnimationsDialog::_context_duplicate_idle_tree() {
   auto* widget    = this->ui.idles;
   auto* model     = (IdleAnimationFormsModel_2*)widget->model();
   auto* sel_model = widget->selectionModel();

   auto qmi = _get_selected_row();
   if (!qmi.isValid())
      return;
   if (!model->canEverDuplicateIdle(qmi))
      return;

   // TODO: This function should be allowed to throw form-creation exceptions, so we can 
   // catch them here and show error messages to the user. Currently, it just swallows 
   // exceptions and returns an intentionally invalid QMI.
   auto idle_qmi = model->duplicateIdle(qmi, true);
   if (!idle_qmi.isValid()) {
      return;
   }

   widget->scrollTo(idle_qmi); // also expands the treeview as necessary
   auto tl = idle_qmi.siblingAtColumn(0);
   auto br = idle_qmi.siblingAtColumn(model->columnCount({}) - 1);
   sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect); // select new idle
}