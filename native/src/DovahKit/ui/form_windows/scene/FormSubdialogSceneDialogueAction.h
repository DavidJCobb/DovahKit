#pragma once
#include <string>
#include <QDialog>
#include <QMenu>
#include "ui_FormSubdialogSceneDialogueAction.h" // generated
#include "dovah/data/dialogue/emotion.h"
#include "./FormSubdialogSceneActionBase.h"
#include "./SceneFormVisualEditor_impl/DialogueAction.h"

namespace dovah {
   class form_stub;
}
class QuestAllDialogueDatastore;
class QuestDialogueTopicInfosModel;

class FormSubdialogSceneDialogueAction : public FormSubdialogSceneActionBase {
   Q_OBJECT;
   public:
      FormSubdialogSceneDialogueAction(QuestAllDialogueDatastore&, QWidget* parent = nullptr);

      SceneFormVisualEditor_impl::DialogueActionData data;
      dovah::form_stub* show_info_on_open = nullptr;

      // After you write to `data`, call this to push state to the UI.
      virtual void refresh() override;

      void select_info(dovah::form_stub*);
      
   protected:
      Ui::FormSubdialogSceneDialogueAction ui;

      QuestAllDialogueDatastore&    dialogue_datastore;
      QuestDialogueTopicInfosModel* info_model = nullptr;
      struct {
         QMenu menu;
         struct {
            QAction* create    = nullptr;
            QAction* edit      = nullptr;
            QAction* move_up   = nullptr;
            QAction* move_down = nullptr;
            QAction* remove    = nullptr;
            QAction* use_info  = nullptr;
         } actions;
      } context_menu;

      dovah::form_stub* _selected_info() const;
      //
      void _info_button_new();
      void _info_button_edit();
      void _info_button_move_up();
      void _info_button_move_down();
      void _info_button_delete();

      bool _did_first_show = false;
      virtual void showEvent(QShowEvent* event) override;
};