#include "./story_manager_event_node.h"
#include "dovah/core.h"
#include "dovah/data/story_manager.h"
#include "editor/helpers/story_event_name.h"
#include "editor/subsystems/story_manager/core.h"
#include "editor/subsystems/story_manager/StoryManagerFormsModel.h"
#include "editor/form_stub_meta_type.h"
#include "ui/models/DKScopedProxyModel.h"
#include "ui/utils/bind.h"

#include "dovah/forms/StoryManagerBranchNode.h"
#include "dovah/forms/StoryManagerEventNode.h"
#include "dovah/forms/StoryManagerQuestNode.h"

FormDialogStoryManagerNodes::FormDialogStoryManagerNodes(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   // The form-dialog mixins require there to be a "cancel" button, but we can't 
   // reasonably offer one because edits are made across forms and in real-time.
   this->ui.buttonCancel->setEnabled(false);
   this->ui.buttonCancel->setVisible(false);

   {
      auto* widget = this->ui.eventType;
      widget->clear();
      for (const auto event : dovah::all_story_event_codes) {
         auto name = editor_helpers::story_event_name(event);
         widget->addItem(name, (int)event);
      }
      widget->model()->sort(0);
   }

   {
      auto* proxy = new DKScopedProxyModel(this->ui.tree);
      auto* model = dovahkit::subsystems::story_manager::core::get_or_create().model();
      proxy->setSourceModel(model);
      proxy->setScopeVisible(true);
      {
         auto qmi = model->index(stub);
         if (qmi.isValid())
            proxy->setScopeIndex(qmi);
      }
      this->ui.tree->setModel(proxy);

      auto* sel_model = this->ui.tree->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, &FormDialogStoryManagerNodes::_pull_selected_node_to_ui);

      QObject::connect(this->ui.nodeEditorID,               &QLineEdit::textChanged,                 this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.nodeFlagRandom,             &QRadioButton::toggled,                  this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.nodeFlagStacked,            &QRadioButton::toggled,                  this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.nodeFlagWarnIfNoStart,      &QCheckBox::toggled,                     this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.questFieldHoursUntilReset,  qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.questFlagResetAfter24Hours, &QCheckBox::toggled,                     this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.nodeFlagDoAll,              &QCheckBox::toggled,                     this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.nodeFlagShares,             &QCheckBox::toggled,                     this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.nodeFlagNumToRun,           &QCheckBox::toggled,                     this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.nodeFlagMaxConcurrent,      &QCheckBox::toggled,                     this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.nodeNumToRun,               qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.nodeMaxConcurrent,          qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
      QObject::connect(this->ui.nodeConditions,             &DKConditionList::changed,               this, &FormDialogStoryManagerNodes::_push_selected_node_from_ui);
   }

   QObject::connect(this->ui.treeExpandAll, &QPushButton::clicked, this->ui.tree, &QTreeView::expandAll);
   QObject::connect(this->ui.treeCollapseAll, &QPushButton::clicked, this->ui.tree, &QTreeView::collapseAll);

   this->load(); // this creates the working copy.
}

void FormDialogStoryManagerNodes::focusForm(const dovah::form_stub& stub) {
   auto* model = dovahkit::subsystems::story_manager::core::get().model();
   auto* proxy = qobject_cast<QAbstractProxyModel*>(this->ui.tree->model());
   assert(!!proxy);
   auto qmi = model->index(stub);
   if (qmi.isValid()) {
      qmi = proxy->mapFromSource(qmi);
      if (qmi.isValid()) {
         this->ui.tree->scrollTo(qmi); // also expands the treeview as necessary
         this->ui.tree->selectionModel()->select({qmi, qmi}, QItemSelectionModel::SelectionFlag::ClearAndSelect);
      }
   }
}

void FormDialogStoryManagerNodes::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   {
      auto* widget = this->ui.eventType;
      widget->setEnabled(false);

      auto i = widget->findData((int)working.event);
      widget->setCurrentIndex(i);
   }
}
void FormDialogStoryManagerNodes::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_push_selected_node_from_ui();
}

QModelIndex FormDialogStoryManagerNodes::_selected_qmi() const noexcept {
   auto* sel_model = this->ui.tree->selectionModel();
   auto  sel       = sel_model->selection();
   if (sel.isEmpty())
      return {};
   return sel[0].topLeft();
}
void FormDialogStoryManagerNodes::_pull_selected_node_to_ui() {
   const auto* model    = dovahkit::subsystems::story_manager::core::get().model();
   const auto  blockers = std::array{
      QSignalBlocker(this->ui.nodeEditorID),
      QSignalBlocker(this->ui.nodeFlagRandom),
      QSignalBlocker(this->ui.nodeFlagStacked),
      QSignalBlocker(this->ui.nodeFlagWarnIfNoStart),
      QSignalBlocker(this->ui.questFieldHoursUntilReset),
      QSignalBlocker(this->ui.questFlagResetAfter24Hours),
      QSignalBlocker(this->ui.nodeFlagDoAll),
      QSignalBlocker(this->ui.nodeFlagShares),
      QSignalBlocker(this->ui.nodeFlagNumToRun),
      QSignalBlocker(this->ui.nodeFlagMaxConcurrent),
      QSignalBlocker(this->ui.nodeNumToRun),
      QSignalBlocker(this->ui.nodeMaxConcurrent),
   };

   dovah::form_stub* stub = nullptr;
   auto qmi = _selected_qmi();
   if (qmi.isValid()) {
      stub = model->data(qmi, StoryManagerFormsModel::FormStubRole).value<dovah::form_stub*>();
      switch (stub->form_type) {
         case dovah::form_type::quest:
         case dovah::form_type::story_branch_node:
         case dovah::form_type::story_event_node:
         case dovah::form_type::story_quest_node:
            break;
         default:
            stub = nullptr;
            break;
      }
   }

   this->ui.itemGroupbox->setEnabled(!!stub);
   if (!stub) {
      this->ui.itemFieldsTyped->setCurrentWidget(this->ui.nodeFields);
      this->ui.nodeEditorID->setText({});
      this->ui.nodeFlagRandom->setChecked(false);
      this->ui.nodeFlagStacked->setChecked(true);
      this->ui.nodeFlagWarnIfNoStart->setChecked(false);
      this->ui.nodeFieldsQuestList->setVisible(false);
      this->ui.nodeConditions->clear();
      return;
   }
   this->ui.nodeEditorID->setText(QString::fromStdString(stub->editorID));

   if (stub->form_type == dovah::form_type::quest) {
      this->ui.nodeEditorID->setEnabled(false);
      this->ui.itemFieldsTyped->setCurrentWidget(this->ui.questFields);

      this->ui.questFieldHoursUntilReset->setValue(model->data(qmi, StoryManagerFormsModel::QuestHoursUntilResetRole).toInt());
      this->ui.questFlagResetAfter24Hours->setChecked(model->data(qmi, StoryManagerFormsModel::QuestResetAfter24HoursRole).toBool());
   } else {
      this->ui.nodeEditorID->setEnabled(true);
      this->ui.itemFieldsTyped->setCurrentWidget(this->ui.nodeFields);

      auto  loaded_base  = stub->load();
      auto* loaded_mixin = dynamic_cast<loaded_node_base_type*>(&*loaded_base);
      assert(!!loaded_mixin);

      this->ui.nodeFlagRandom->setChecked(loaded_mixin->flags & loaded_node_base_type::flag::random);
      this->ui.nodeFlagWarnIfNoStart->setChecked(loaded_mixin->flags & loaded_node_base_type::flag::warn_if_no_child_quest_started);
      this->ui.nodeConditions->importFrom(*loaded_base, loaded_mixin->conditions);
      if (stub->form_type == dovah::form_type::story_quest_node) {
         auto* loaded_qnode = dynamic_cast<dovah::loaded_forms::StoryManagerQuestNode*>(&*loaded_base);
         assert(!!loaded_qnode);
         this->ui.nodeFieldsQuestList->setVisible(true);
         this->ui.nodeFlagDoAll->setChecked(loaded_qnode->flags & loaded_node_base_type::flag::do_all_before_repeating);
         this->ui.nodeFlagShares->setChecked(loaded_qnode->flags & loaded_node_base_type::flag::shares_event);
         this->ui.nodeFlagNumToRun->setChecked(loaded_qnode->flags & loaded_node_base_type::flag::num_quests_to_run);
         this->ui.nodeFlagMaxConcurrent->setChecked(loaded_qnode->max_concurrent_quests != 0);
         this->ui.nodeNumToRun->setValue(loaded_qnode->num_quests_to_run);
         this->ui.nodeMaxConcurrent->setValue(loaded_qnode->max_concurrent_quests);

         this->ui.nodeNumToRun->setEnabled(this->ui.nodeFlagNumToRun->isChecked());
         this->ui.nodeMaxConcurrent->setEnabled(this->ui.nodeFlagMaxConcurrent->isChecked());
      } else {
         this->ui.nodeFieldsQuestList->setVisible(false);
      }
   }
}
void FormDialogStoryManagerNodes::_push_selected_node_from_ui() {
   auto* model = dovahkit::subsystems::story_manager::core::get().model();

   dovah::form_stub* stub = nullptr;
   auto qmi = _selected_qmi();
   if (qmi.isValid()) {
      stub = model->data(qmi, StoryManagerFormsModel::FormStubRole).value<dovah::form_stub*>();
      switch (stub->form_type) {
         case dovah::form_type::quest:
         case dovah::form_type::story_branch_node:
         case dovah::form_type::story_event_node:
         case dovah::form_type::story_quest_node:
            break;
         default:
            stub = nullptr;
            break;
      }
   }
   if (!stub)
      return;

   if (stub->form_type == dovah::form_type::quest) {
      model->setData(qmi, this->ui.questFieldHoursUntilReset->value(), StoryManagerFormsModel::QuestHoursUntilResetRole);
      model->setData(qmi, this->ui.questFlagResetAfter24Hours->isChecked(), StoryManagerFormsModel::QuestResetAfter24HoursRole);
      return;
   }

   auto  loaded_base  = stub->load();
   auto* loaded_mixin = dynamic_cast<loaded_node_base_type*>(&*loaded_base);
   assert(!!loaded_mixin);

   auto& editor = DovahKitCore::get();
   emit editor.formModificationImminent(stub);
   //
   stub->set_edited(true);
   stub->editorID = this->ui.nodeEditorID->text().toStdString();
   cobb::edit_bit(loaded_mixin->flags, loaded_node_base_type::flag::random, this->ui.nodeFlagRandom->isChecked());
   cobb::edit_bit(loaded_mixin->flags, loaded_node_base_type::flag::warn_if_no_child_quest_started, this->ui.nodeFlagWarnIfNoStart->isChecked());
   this->ui.nodeConditions->exportTo(*loaded_base, loaded_mixin->conditions);
   if (stub->form_type == dovah::form_type::story_quest_node) {
      auto* loaded_qnode = dynamic_cast<dovah::loaded_forms::StoryManagerQuestNode*>(&*loaded_base);
      assert(!!loaded_qnode);
      cobb::edit_bit(loaded_qnode->flags, loaded_node_base_type::flag::do_all_before_repeating, this->ui.nodeFlagDoAll->isChecked());
      cobb::edit_bit(loaded_qnode->flags, loaded_node_base_type::flag::shares_event,            this->ui.nodeFlagShares->isChecked());
      cobb::edit_bit(loaded_qnode->flags, loaded_node_base_type::flag::num_quests_to_run,       this->ui.nodeFlagNumToRun->isChecked());
      loaded_qnode->num_quests_to_run = this->ui.nodeNumToRun->value();
      if (this->ui.nodeFlagMaxConcurrent->isChecked()) {
         loaded_qnode->max_concurrent_quests = this->ui.nodeMaxConcurrent->value();
      } else {
         loaded_qnode->max_concurrent_quests = 0;
      }
   }
   //
   emit editor.formModified(stub);
}