#pragma once
#include "./_base.h"
#include "dovah/forms/Quest.h"
#include "ui_quest.h"

class DKQuestSceneEditor;

class QuestTabStages;
class QuestTabObjectives;
class QuestTabScenes;

class QuestAllDialogueDatastore;
class QuestDialogueTabBody;

class FormDialogQuest :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Quest, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogQuest(dovah::form_stub& stub, QWidget* parent = Q_NULLPTR);
      
   protected:
      Ui::FormDialogQuest ui;
      struct {
         QuestAllDialogueDatastore* dialogue_datastore = nullptr;
      } data;
      struct {
         QuestTabStages*     stages     = nullptr;
         QuestTabObjectives* objectives = nullptr;
         QuestTabScenes*     scenes     = nullptr;
      } tabs;
      struct {
         struct {
            QuestDialogueTabBody* player    = nullptr;
            QuestDialogueTabBody* favor_a   = nullptr;
            QuestDialogueTabBody* combat    = nullptr;
            QuestDialogueTabBody* favor_b   = nullptr;
            QuestDialogueTabBody* detection = nullptr;
            QuestDialogueTabBody* services  = nullptr;
            QuestDialogueTabBody* misc      = nullptr;
         } dialogue_tab_bodies;
         DKQuestSceneEditor* scene_editor = nullptr;
      } subwidgets;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

   public:
      void focus_dialogue_branch(dovah::form_stub&);
      void focus_dialogue_topic(dovah::form_stub& topic, dovah::form_stub* info = nullptr);
      void focus_dialogue_info(dovah::form_stub&);
};
