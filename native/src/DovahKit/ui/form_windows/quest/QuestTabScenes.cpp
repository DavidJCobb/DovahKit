#include "./QuestTabScenes.h"
#include <array>
#include "dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "dovah/forms/Scene.h"
#include "dovah/forms/Quest.h"
#include "editor/core.h"
#include "../scene/SceneFormVisualEditor.h"

#include <QBoxLayout>

QuestTabScenes::QuestTabScenes(quest_form_type& quest, QuestAllDialogueDatastore& ds, QWidget* parent) : QObject(parent), dialogue_datastore(ds), working_quest(quest) {
}
QuestTabScenes::~QuestTabScenes() {
   if (auto* wc = this->_state.loaded_scene) {
      this->_state.loaded_scene = nullptr;
      //
      // Compare to FormEditDialogMixin.
      //
      auto& editor = DovahKitCore::get();
      auto& stub   = wc->stub;
      emit editor.formWorkingCopyDeleteImminent(&stub);
      stub.delete_working_copy();
      emit editor.formWorkingCopyDeleteComplete(&stub);
   }
}
void QuestTabScenes::setupUi() {
   this->ui.scene_picker->setAllowedFormTypes({ dovah::form_type::scene });
   this->ui.scene_picker->setAllowMultiSelect(false);
   this->ui.scene_picker->setReadOnly(true);

   auto* scene_edit_widget = this->ui.current_scene.editor = new SceneFormVisualEditor(this->ui.current_scene.editor_scrollbox);
   //this->ui.current_scene.editor_scrollbox->setWidget(scene_edit_widget);
   {
      auto* wrapper = new QWidget(this->ui.current_scene.editor_scrollbox);
      auto* layout = new QHBoxLayout(wrapper);
      layout->addWidget(scene_edit_widget);
      layout->setContentsMargins(0, 0, 0, 0);
      this->ui.current_scene.editor_scrollbox->setWidget(wrapper);
   }

   QObject::connect(this->ui.scene_picker, &DKFormListPane::selectedFormsChanged, this, [this](const std::vector<dovah::form_stub*>& forms) {
      if (forms.empty()) {
         this->select_scene(nullptr);
      } else {
         this->select_scene(forms[0]);
      }
   });
   QObject::connect(this->ui.current_scene.buttons.actor_behavior,      &QPushButton::clicked, scene_edit_widget, &SceneFormVisualEditor::popBehaviorFlagDialog);
   QObject::connect(this->ui.current_scene.buttons.actor_participation, &QPushButton::clicked, scene_edit_widget, &SceneFormVisualEditor::popParticipationFlagDialog);

   for (auto& use : this->working_quest.stub.inbound) {
      if (use.second.flags & dovah::use_info_entry::flag::dialogue_quest) {
         auto* stub = use.second.other;
         if (stub->form_type != dovah::form_type::scene)
            continue;
         this->ui.scene_picker->addStub(stub);
      }
   }

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formCreated, this, &QuestTabScenes::_on_form_created);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &QuestTabScenes::_on_form_deleted);
}

void QuestTabScenes::focus_dialogue_forms(uint32_t action_id, dovah::form_stub* topic, dovah::form_stub* info) {
   this->ui.current_scene.editor->focus_dialogue_forms(action_id, topic, info);
}
void QuestTabScenes::select_scene(dovah::form_stub* stub) {
   this->push_data_to_scene_form(); // commit data to scenes when we switch

   auto* widget = this->ui.current_scene.editor;
   if (!stub || stub->form_type != dovah::form_type::scene) {
      this->_state.loaded_scene = {};
      widget->clear();
      widget->update();
      return;
   }
   this->_state.loaded_scene = (scene_form_type*) stub->create_working_copy();
   if (!this->_state.loaded_scene) {
      this->_state.loaded_scene = {};
      widget->clear();
      widget->update();
      return;
   }

   SceneFormVisualEditor::SceneContext context = {
      .quest    = &this->working_quest.stub,
      .scene    = stub,
      .dialogue = &this->dialogue_datastore,
   };
   widget->setContext(context);
   widget->importData();

   auto& subwidgets = this->ui.current_scene;

   const auto blockers = std::array{
      QSignalBlocker(subwidgets.editor_id),
      QSignalBlocker(subwidgets.flags.start_scene_with_quest),
      QSignalBlocker(subwidgets.flags.end_quest_with_scene),
   };
   subwidgets.editor_id->setText(QString::fromStdString(stub->editorID));
   subwidgets.flags.start_scene_with_quest->setChecked(this->_state.loaded_scene->scene_flags & scene_form_type::scene_flag::begin_on_quest_start);
   subwidgets.flags.end_quest_with_scene->setChecked(this->_state.loaded_scene->scene_flags & scene_form_type::scene_flag::stop_quest_on_end);
}
void QuestTabScenes::push_data_to_scene_form() {
   if (!this->_state.loaded_scene)
      return;

   auto& working = *this->_state.loaded_scene;
   auto& stub    = working.stub;

   if (!this->ui.current_scene.editor->hasAnyChanges()) {
      bool changed = false;
      {
         auto prior = working.scene_flags & scene_form_type::scene_flag::begin_on_quest_start;
         auto after = this->ui.current_scene.flags.start_scene_with_quest->isChecked();
         if (prior != after)
            changed = true;
      }
      if (!changed) {
         auto prior = working.scene_flags & scene_form_type::scene_flag::stop_quest_on_end;
         auto after = this->ui.current_scene.flags.end_quest_with_scene->isChecked();
         if (prior != after)
            changed = true;
      }
      if (!changed) {
         auto prior = stub.editorID;
         auto after = this->ui.current_scene.editor_id->text().toStdString();
         if (prior != after)
            changed = true;
      }
      if (!changed) {
         return;
      }
   }

   //
   // Same basic steps as FormEditDialogMixin::save.
   //
   auto& editor = DovahKitCore::get();
   emit editor.formWorkingCopyCommitImminent(&stub);
   emit editor.formModificationImminent(&stub);
   stub.set_edited(true);
   {  // actual changes
      stub.editorID = this->ui.current_scene.editor_id->text().toStdString();
      cobb::edit_bit(working.scene_flags, scene_form_type::scene_flag::begin_on_quest_start, this->ui.current_scene.flags.start_scene_with_quest->isChecked());
      cobb::edit_bit(working.scene_flags, scene_form_type::scene_flag::stop_quest_on_end,    this->ui.current_scene.flags.end_quest_with_scene->isChecked());
      this->ui.current_scene.editor->exportData();
   }
   stub.commit_working_copy();
   emit editor.formWorkingCopyCommitComplete(&stub);
   emit editor.formModified(&stub);
}

void QuestTabScenes::_on_form_created(dovah::form_stub* stub) {
   if (stub->form_type != dovah::form_type::scene)
      return;

   auto* quest = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info_entry::flag::dialogue_quest>(*stub);
   if (!quest)
      return;
   if (quest != &this->working_quest.stub)
      return;

   auto* widget = this->ui.scene_picker;
   if (widget->contains(stub))
      return;
   widget->addStub(stub);
}
void QuestTabScenes::_on_form_deleted(dovah::form_stub* stub, bool just_being_flagged) {
   if (stub == &this->_state.loaded_scene->stub) {
      this->_state.loaded_scene = nullptr; // so we don't do anything in `push_data_to_scene_form`
      this->select_scene(nullptr);
   }
   this->ui.scene_picker->removeStub(stub);
}