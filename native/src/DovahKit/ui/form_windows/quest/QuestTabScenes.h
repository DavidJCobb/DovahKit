#pragma once
#include <cstdint>
#include <QAction>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTableView>
#include "dovah/form_stub.h"
#include "widgets/DKFormListPane.h"

namespace dovah {
   namespace loaded_forms {
      class Scene;
      class Quest;
   }
   class form_stub;
}
class QuestAllDialogueDatastore;
class SceneFormVisualEditor;

class QuestTabScenes : public QObject {
   Q_OBJECT;
   private:
      using scene_form_type = dovah::loaded_forms::Scene;
      using quest_form_type = dovah::loaded_forms::Quest;
   public:
      QuestTabScenes(quest_form_type& quest, QuestAllDialogueDatastore&, QWidget* parent = nullptr);
      ~QuestTabScenes();

      void setupUi();

      struct {
         struct {
            QPushButton* zoom_in  = nullptr;
            QPushButton* zoom_out = nullptr;
         } buttons;
         QCheckBox* show_all_text = nullptr;

         DKFormListPane* scene_picker = nullptr;

         struct {
            struct {
               QPushButton* actor_behavior      = nullptr;
               QPushButton* actor_participation = nullptr;
            } buttons;
            QLineEdit* editor_id = nullptr;
            struct {
               QCheckBox* start_scene_with_quest = nullptr;
               QCheckBox* end_quest_with_scene   = nullptr;
            } flags;
            QScrollArea* editor_scrollbox = nullptr;
            SceneFormVisualEditor* editor = nullptr;
         } current_scene;
      } ui;

      void select_scene(dovah::form_stub*);
      void push_data_to_scene_form();
      
   protected:
      QuestAllDialogueDatastore& dialogue_datastore;
      quest_form_type& working_quest;
      struct {
         scene_form_type* loaded_scene = nullptr; // working copy
      } _state;

      struct {
         struct {
            QAction* insert = nullptr;
            QAction* remove = nullptr;
         } objective_list;
         struct {
            QAction* insert   = nullptr;
            QAction* remove   = nullptr;
            QAction* moveUp   = nullptr;
            QAction* moveDown = nullptr;
         } target_list;
      } context_menu_actions;
      
      void _on_form_created(dovah::form_stub*);
      void _on_form_deleted(dovah::form_stub*, bool just_being_flagged);
};
