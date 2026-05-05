#include "./story_manager_event_node.h"
#include <QInputDialog>
#include "dovah/data/story_manager.h"
#include "editor/helpers/story_event_name.h"
#include "editor/subsystems/story_manager/core.h"
#include "editor/subsystems/story_manager/StoryManagerFormsModel.h"
#include "editor/form_stub_meta_type.h"
#include "ui/models/DKScopedProxyModel.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_custom_context_menu.h"
#include "widgets/DKFormPickerDialog.h"

#include "dovah/forms/Quest.h" // for QUST-side event conditions
#include "dovah/forms/StoryManagerBranchNode.h"
#include "dovah/forms/StoryManagerEventNode.h"
#include "dovah/forms/StoryManagerQuestNode.h"

#include "./story_manager_event_node/StoryManagerQuestNodeAddQuestsFilter.h"

#include "editor/open_window_for_form.h"

namespace {
   constexpr const bool do_not_allow_creating_multiple_nodes_for_one_event = true;
}

FormDialogStoryManagerNodes::FormDialogStoryManagerNodes(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   // The form-dialog mixins require there to be both an "OK" button and a 
   // "Cancel" button, but we can't reasonably offer a true "cancel" button 
   // because edits are made across multiple forms and in real-time. So, 
   // we have to hide the "OK" button.
   //
   // At the same time: we don't want the mere act of clicking "OK" on this 
   // dialog to create an SMEN ITM. The SMEN should only be flagged as 
   // "edited" if you actually edit it via this UI. (The nuance here is that 
   // this UI is "dedicated" to a single form, the SMEN, but is used to edit 
   // multiple forms which may or may not include that SMEN.)
   //
   // So here's the hack. We'll hide the "cancel" button, and rename "OK" 
   // to say "cancel." That way, clicking the button won't blindly save 
   // [a total lack of] changes to the SMEN (because under the hood, it 
   // "cancels" changes), but the user will also see that there's no true 
   // way to "cancel."
   {
      auto* cancel = this->ui.buttonCancel;
      auto* commit = this->ui.buttonOK;
      cancel->setText(commit->text());
      commit->setEnabled(false);
      commit->setVisible(false);
      commit->setText(tr("Wait, what? How can you see this?"));
   }

   {
      auto* widget = this->ui.eventType;
      widget->clear();
      for (const auto event : dovah::all_story_event_codes) {
         if (event == dovah::story_event_code::none)
            continue;
         if (event == dovah::story_event_code::undefined)
            continue;
         auto name = editor_helpers::story_event_name(event);
         widget->addItem(name, (int)event);
      }
      widget->model()->sort(0);
      widget->insertItem(0, tr("NONE"), 0);
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
      this->ui.breadcrumbs->setModel(proxy);
      this->ui.breadcrumbs->setSegmentNameRole(StoryManagerFormsModel::EditorIDRole);

      this->ui.tree->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
      this->ui.tree->setDragDropOverwriteMode(false);
      this->ui.tree->setDragEnabled(true);
      this->ui.tree->setAcceptDrops(true);
      this->ui.tree->setDropIndicatorShown(true);

      auto* sel_model = this->ui.tree->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, &FormDialogStoryManagerNodes::_pull_selected_node_to_ui);
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& sel) {
         QModelIndex qmi;
         if (!sel.empty())
            qmi = sel[0].topLeft();
         this->ui.breadcrumbs->setCurrentIndex(qmi);
      });

      QObject::connect(this->ui.nodeFlagMaxConcurrent, &QCheckBox::toggled, this->ui.nodeMaxConcurrent, &QWidget::setEnabled);
      QObject::connect(this->ui.nodeFlagNumToRun,      &QCheckBox::toggled, this->ui.nodeNumToRun, &QWidget::setEnabled);

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

   // context menu
   {
      auto& menu    = this->context.menu;
      auto& actions = this->context.actions;
      ui::set_custom_context_menu(*this->ui.tree, menu);
      {
         auto* action = actions.node.branch.new_branch_node = new QAction(tr("Create branch node..."), this);
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, &FormDialogStoryManagerNodes::_context_branch_new_child_branch);
      }
      {
         auto* action = actions.node.branch.new_quest_node = new QAction(tr("Create quest node..."), this);
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, &FormDialogStoryManagerNodes::_context_branch_new_child_quest_list);
      }
      {
         auto* action = actions.node.quest.add_quests = new QAction(tr("Add quests..."), this);
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, &FormDialogStoryManagerNodes::_context_quest_list_add_quests);
      }
      {
         auto* action = actions.node.delete_node = new QAction(tr("Delete node"), this);
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, &FormDialogStoryManagerNodes::_context_delete_node);
      }

      {
         auto* action = actions.quest_form.edit = new QAction(tr("Edit quest..."), this);
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, &FormDialogStoryManagerNodes::_context_quest_form_edit);
      }
      {
         auto* action = actions.quest_form.remove = new QAction(tr("Remove quest from node"), this);
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, &FormDialogStoryManagerNodes::_context_quest_form_remove);
      }

      {
         auto* action = actions.use_info = new QAction(tr("Use Info"), this);
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, &FormDialogStoryManagerNodes::_context_use_info);
      }

      QObject::connect(&menu, &QMenu::aboutToShow, this, [this]() {
         auto* model = dovahkit::subsystems::story_manager::core::get().model();
         auto  qmi   = _selected_qmi();
         auto* form  = model->data(qmi, StoryManagerFormsModel::FormStubRole).value<dovah::form_stub*>();

         bool is_branch     = false;
         bool is_quest_list = false;
         bool is_quest_form = false;
         if (form) {
            switch (form->form_type) {
               case dovah::form_type::story_event_node:
               case dovah::form_type::story_branch_node:
                  is_branch = true;
                  break;
               case dovah::form_type::story_quest_node:
                  is_quest_list = true;
                  break;
               case dovah::form_type::quest:
                  is_quest_form = true;
                  break;
            }
         }

         this->context.actions.node.branch.new_branch_node->setVisible(is_branch);
         this->context.actions.node.branch.new_quest_node->setVisible(is_branch);
         this->context.actions.node.quest.add_quests->setVisible(is_quest_list);
         this->context.actions.node.delete_node->setVisible(form && !is_quest_form);

         this->context.actions.quest_form.edit->setVisible(is_quest_form);
         this->context.actions.quest_form.remove->setVisible(is_quest_form);

         this->context.actions.use_info->setVisible(form != nullptr);
      });
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

      bool has_event = true;
      switch (working.event) {
         case dovah::story_event_code::none:
         case dovah::story_event_code::undefined:
            has_event = false;
            break;
      }

      if (has_event) {
         widget->setEnabled(false);
         this->ui.tree->setEnabled(true);
      } else {
         widget->setEnabled(true);
         this->ui.tree->setEnabled(false);

         if constexpr (do_not_allow_creating_multiple_nodes_for_one_event) {
            //
            // Remove from the combobox all event types that are already being 
            // used by another form.
            //
            auto& editor = DovahKitCore::get();
            auto& sm     = dovahkit::subsystems::story_manager::core::get();
            editor.for_each_form_of_type(dovah::form_type::story_event_node, [&sm, widget](dovah::form_stub* stub) -> bool {
               auto ev_opt = sm.event_type_for(*stub);
               if (!ev_opt.has_value())
                  return false;
               auto ev = ev_opt.value();
               if (ev == 0)
                  return false;
               auto i = widget->findData((int)ev_opt.value());
               if (i >= 0)
                  widget->removeItem(i);
               return false;
            });
         }

         QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
            auto et = this->ui.eventType->currentData().toInt();
            if (!et)
               return;

            //
            // We want to make this change immediately so that the SM event 
            // subsystem can understand what's going on, given that all other 
            // changes to SM nodes are immediate as well.
            //

            auto& editor = DovahKitCore::get();
            emit editor.formModificationImminent(&this->form->stub);
            //
            this->form->event = et;
            // `this->form` is just the working copy; we want to edit the "real" 
            // form.
            auto actual = this->form->stub.load().ptr_cast<loaded_form_type>();
            actual->event = et;
            //
            this->form->stub.set_edited(true);
            emit editor.formModified(&this->form->stub);

            //
            // Finally, disable changing the event type, and enable changing 
            // the event contents.
            //

            this->ui.eventType->setEnabled(false);
            this->ui.tree->setEnabled(true);
         });
      }
      auto i = widget->findData((int)working.event);
      if (i < 0 && !has_event)
         i = 0; // "NONE"
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
   return ((QAbstractProxyModel*)this->ui.tree->model())->mapToSource(sel[0].topLeft());
}
std::pair<QModelIndex, dovah::form_stub*> FormDialogStoryManagerNodes::_selected_source_model_item() const noexcept {
   auto  qmi  = _selected_qmi();
   auto* stub = qmi.data(StoryManagerFormsModel::FormStubRole).value<dovah::form_stub*>();
   return { qmi, stub };
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

      auto v_opt = model->questProperties(qmi);
      if (v_opt.has_value()) {
         auto& v = v_opt.value();
         this->ui.questFieldHoursUntilReset->setValue(v.hours_until_reset);
         this->ui.questFlagResetAfter24Hours->setChecked(v.reset_after_24_hours);
      }

      auto loaded_ptr = stub->load().ptr_cast<dovah::loaded_forms::Quest>();
      if (loaded_ptr) {
         this->ui.nodeConditions->importFrom(*loaded_ptr, loaded_ptr->conditions.event);
      }
   } else {
      this->ui.nodeEditorID->setEnabled(true);
      this->ui.itemFieldsTyped->setCurrentWidget(this->ui.nodeFields);

      auto  loaded_base  = stub->load();
      auto* loaded_mixin = dynamic_cast<loaded_node_base_type*>(&*loaded_base);
      assert(!!loaded_mixin);

      this->ui.nodeFlagStacked->setChecked(!(loaded_mixin->flags & loaded_node_base_type::flag::random));
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

   auto loaded_base = stub->load();
   if (stub->form_type == dovah::form_type::quest) {
      StoryManagerFormsModel::quest_properties v = {
         .hours_until_reset    = (float)this->ui.questFieldHoursUntilReset->value(),
         .reset_after_24_hours = this->ui.questFlagResetAfter24Hours->isChecked(),
      };
      model->setQuestProperties(qmi, v);

      auto loaded_quest = loaded_base.ptr_cast<dovah::loaded_forms::Quest>();
      assert(!!loaded_quest);

      auto& editor = DovahKitCore::get();
      emit editor.formModificationImminent(stub);
      stub->set_edited(true);
      //
      this->ui.nodeConditions->exportTo(*loaded_quest, loaded_quest->conditions.event);
      //
      emit editor.formModified(stub);
      return;
   }

   auto* loaded_mixin = dynamic_cast<loaded_node_base_type*>(&*loaded_base);
   assert(!!loaded_mixin);

   auto& editor = DovahKitCore::get();
   emit editor.formModificationImminent(stub);
   stub->set_edited(true);
   //
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

void FormDialogStoryManagerNodes::_focus_qmi(const QModelIndex& qmi) {
   if (!qmi.isValid())
      return;
   auto* model = dovahkit::subsystems::story_manager::core::get().model();
   auto* proxy = qobject_cast<QAbstractProxyModel*>(this->ui.tree->model());
   assert(!!model);
   assert(!!proxy);
   assert(qmi.model() == proxy || qmi.model() == model);
   if (qmi.model() == model) {
      auto proxy_qmi = proxy->mapFromSource(qmi);
      if (proxy_qmi.isValid()) {
         this->ui.tree->scrollTo(proxy_qmi); // also expands the treeview as necessary
         this->ui.tree->selectionModel()->select({ proxy_qmi, proxy_qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
      }
   } else {
      this->ui.tree->scrollTo(qmi); // also expands the treeview as necessary
      this->ui.tree->selectionModel()->select({ qmi, qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
   }
}

#pragma region Context menu
   void FormDialogStoryManagerNodes::_context_branch_new_child_branch() {
      auto [parent_qmi, form] = _selected_source_model_item();
      if (!form)
         return;
      switch (form->form_type) {
         case dovah::form_type::story_branch_node:
         case dovah::form_type::story_event_node:
            break;
         default:
            return;
      }

      bool ok;
      auto edid = QInputDialog::getText(
         this,
         tr("New branch node"),
         tr("Editor ID:"),
         QLineEdit::Normal,
         {},
         &ok
      );
      if (!ok || edid.isEmpty())
         return;

      auto* model       = dovahkit::subsystems::story_manager::core::get().model();
      auto  created_qmi = model->createBranchIn(parent_qmi, edid);
      _focus_qmi(created_qmi);
   }
   void FormDialogStoryManagerNodes::_context_branch_new_child_quest_list() {
      auto [parent_qmi, form] = _selected_source_model_item();
      if (!form)
         return;
      switch (form->form_type) {
         case dovah::form_type::story_branch_node:
         case dovah::form_type::story_event_node:
            break;
         default:
            return;
      }

      bool ok;
      auto edid = QInputDialog::getText(
         this,
         tr("New quest node"),
         tr("Editor ID:"),
         QLineEdit::Normal,
         {},
         &ok
      );
      if (!ok || edid.isEmpty())
         return;

      auto* model       = dovahkit::subsystems::story_manager::core::get().model();
      auto  created_qmi = model->createQuestListIn(parent_qmi, edid);
      _focus_qmi(created_qmi);
   }
   void FormDialogStoryManagerNodes::_context_quest_list_add_quests() {
      auto [parent_qmi, form] = _selected_source_model_item();
      if (!form || form->form_type != dovah::form_type::story_quest_node)
         return;

      auto* dialog = new DKFormPickerDialog(this);
      dialog->setAllowedFormType(dovah::form_type::quest);
      {
         auto* filter = new StoryManagerQuestNodeAddQuestsFilter(dialog);
         {
            auto& sm   = dovahkit::subsystems::story_manager::core::get();
            auto* smen = sm.containing_event_node_of(*form);
            if (smen) {
               auto et_opt = sm.event_type_for(*smen);
               if (et_opt.has_value())
                  filter->setEvent(et_opt.value());
            }
         }
         {
            auto*  model = dovahkit::subsystems::story_manager::core::get_or_create().model();
            size_t count = model->rowCount(parent_qmi);
            if (count > 0) {
               std::vector<dovah::form_stub*> quests_already_present;
               for (size_t i = 0; i < count; ++i) {
                  auto* form = model->index(i, 0, parent_qmi).data(StoryManagerFormsModel::FormStubRole).value<dovah::form_stub*>();
                  if (form)
                     quests_already_present.push_back(form);
               }
               filter->setQuestsToExclude(std::move(quests_already_present));
            }
         }
         dialog->setCustomFilter(filter);
      }
      if (dialog->exec() == QDialog::Accepted) {
         auto* quest = dialog->formStub();
         if (quest) {
            auto* model     = dovahkit::subsystems::story_manager::core::get().model();
            auto  quest_qmi = model->addQuestTo(parent_qmi, *quest);
            _focus_qmi(quest_qmi);
         }
      }
      dialog->deleteLater();
   }
   void FormDialogStoryManagerNodes::_context_delete_node() {
      auto [qmi, form] = _selected_source_model_item();
      if (!form)
         return;
      switch (form->form_type) {
         case dovah::form_type::story_event_node:
         case dovah::form_type::story_branch_node:
         case dovah::form_type::story_quest_node:
            break;
         default:
            return;
      }
      auto* model = dovahkit::subsystems::story_manager::core::get().model();
      model->deleteNode(qmi);
   }
   void FormDialogStoryManagerNodes::_context_quest_form_edit() {
      auto* model = dovahkit::subsystems::story_manager::core::get().model();
      auto  qmi   = _selected_qmi();
      if (!qmi.isValid())
         return;
      auto* form = model->data(qmi, StoryManagerFormsModel::FormStubRole).value<dovah::form_stub*>();
      if (!form || form->form_type != dovah::form_type::quest)
         return;
      open_edit_dialog_for_form(*form);
   }
   void FormDialogStoryManagerNodes::_context_quest_form_remove() {
      auto [qmi, form] = _selected_source_model_item();
      if (!form || form->form_type != dovah::form_type::quest)
         return;
      auto* model = dovahkit::subsystems::story_manager::core::get().model();
      model->removeQuestFromNode(qmi);
   }
   void FormDialogStoryManagerNodes::_context_use_info() {
      auto [qmi, form] = _selected_source_model_item();
      if (!form)
         return;
      open_use_info_dialog_for_form(*form);
   }
#pragma endregion