#pragma once
#include <QMenu>
#include "dovah/data/dialogue/category.h"
#include "ui_quest_dialogue_tab_body.h" // generated

namespace dovah {
   class form_stub;
}
class QuestAllDialogueDatastore;
class QuestDialogueBranchesModel;
class QuestDialogueBranchedTopicsModel;
class QuestDialogueBranchlessTopicsModel;
class QuestDialogueTopicInfosModel;

class QuestDialogueTabBody : public QWidget {
   Q_OBJECT;
   public:
      QuestDialogueTabBody(QWidget* parent = nullptr);

      QuestAllDialogueDatastore* datastore() const;

      void setCategory(dovah::dialogue::category);
      void setDatastore(QuestAllDialogueDatastore*);

   protected:
      struct context_menu_definition {
         QMenu menu;
         struct {
            QAction* create     = nullptr;
            QAction* edit       = nullptr;
            QAction* move_up    = nullptr;
            QAction* move_down  = nullptr;
            QAction* transplant = nullptr;
            QAction* remove     = nullptr;
            QAction* use_info   = nullptr;
         } actions;

         void make_for(QWidget&, bool allow_reordering, QString transplant_label = "");
         void update_enable_states(bool has_datastore, bool has_item);
      };

   protected:
      Ui::QuestDialogueTabBody ui;

      dovah::dialogue::category _category = dovah::dialogue::category::topic;
      struct {
         context_menu_definition branches;
         context_menu_definition topics;
         context_menu_definition infos;
      } _context_menus;
      struct {
         QuestDialogueBranchesModel* branches = nullptr;
         struct {
            QuestDialogueBranchedTopicsModel*   branched   = nullptr;
            QuestDialogueBranchlessTopicsModel* branchless = nullptr;
         } topics;
         QuestDialogueTopicInfosModel* infos = nullptr;
      } _models;

      void _set_up_topic_selection_model();

      void _reset_selection_of(QTableView*);

      void _branch_selection_changed();
      void _topic_selection_changed();
      void _info_selection_changed();

      dovah::form_stub* _quest() const;

      // These JUST create and configure the new forms; they don't do anything within the UI.
      dovah::form_stub* _spawn_branch(QString branch_editor_id, QString topic_editor_id);
      dovah::form_stub* _spawn_topic(QString editor_id, dovah::form_stub* branch, uint32_t subtype_signature);
      dovah::form_stub* _spawn_info(dovah::form_stub* topic);

      #pragma region UI button handlers
         #pragma region Branches
            void _branch_button_new();
            void _branch_button_edit();
            void _branch_button_delete();
         #pragma endregion
         #pragma region Topics
            void _topic_button_new();
            void _topic_button_edit();
            void _topic_button_delete();
         #pragma endregion
         #pragma region Infos
            void _info_button_new();
            void _info_button_edit();
            void _info_button_move_up();
            void _info_button_move_down();
            void _info_button_delete();
         #pragma endregion
      #pragma endregion
      
   public:
      dovah::form_stub* selected_branch() const;
      dovah::form_stub* selected_topic() const;
      dovah::form_stub* selected_info() const;

      void select_branch(dovah::form_stub*);
      void select_topic(dovah::form_stub*);
      void select_info(dovah::form_stub*);
};