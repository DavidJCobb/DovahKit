#include "./QuestTabScenes.h"
#include <array>
#include "dovah/forms/Scene.h"
#include "dovah/forms/Quest.h"
#include "../scene/SceneFormVisualEditor.h"

QuestTabScenes::QuestTabScenes(quest_form_type& quest, QuestAllDialogueDatastore& ds, QWidget* parent) : QObject(parent), dialogue_datastore(ds), working_quest(quest) {
}
void QuestTabScenes::setupUi() {
   this->ui.scene_picker->setAllowedFormTypes({ dovah::form_type::scene });
   this->ui.scene_picker->setAllowMultiSelect(false);
   this->ui.scene_picker->setReadOnly(true);

   auto* scene_edit_widget = this->ui.current_scene.editor = new SceneFormVisualEditor(this->ui.current_scene.editor_scrollbox);
   this->ui.current_scene.editor_scrollbox->setWidget(scene_edit_widget);

   QObject::connect(this->ui.scene_picker, &DKFormListPane::selectedFormsChanged, this, [this](const std::vector<dovah::form_stub*>& forms) {
      if (forms.empty()) {
         this->select_scene(nullptr);
      } else {
         this->select_scene(forms[0]);
      }
   });
   QObject::connect(this->ui.current_scene.buttons.actor_behavior,      &QPushButton::clicked, scene_edit_widget, &SceneFormVisualEditor::popBehaviorFlagDialog);
   QObject::connect(this->ui.current_scene.buttons.actor_participation, &QPushButton::clicked, scene_edit_widget, &SceneFormVisualEditor::popParticipationFlagDialog);
}

void QuestTabScenes::select_scene(dovah::form_stub* stub) {
   auto* widget = this->ui.current_scene.editor;
   if (!stub || stub->form_type != dovah::form_type::scene) {
      this->_state.loaded_scene = {};
      widget->clear();
      widget->update();
      return;
   }
   this->_state.loaded_scene = stub->load().ptr_cast<dovah::loaded_forms::Scene>();

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
   if (this->_state.loaded_scene)
      return;

   assert(this->ui.current_scene.editor->scene() == &this->_state.loaded_scene->stub);

   this->_state.loaded_scene->stub.editorID = this->ui.current_scene.editor_id->text().toStdString();
   cobb::edit_bit(this->_state.loaded_scene->scene_flags, scene_form_type::scene_flag::begin_on_quest_start, this->ui.current_scene.flags.start_scene_with_quest->isChecked());
   cobb::edit_bit(this->_state.loaded_scene->scene_flags, scene_form_type::scene_flag::stop_quest_on_end,    this->ui.current_scene.flags.end_quest_with_scene->isChecked());
   this->ui.current_scene.editor->exportData();
}