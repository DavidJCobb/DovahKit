#pragma once
#include <string>
#include <QDialog>
#include "ui_FormSubdialogSceneTimerAction.h" // generated
#include "./FormSubdialogSceneActionBase.h"

namespace dovah::loaded_forms {
   class Scene;
}

class FormSubdialogSceneTimerAction : public FormSubdialogSceneActionBase {
   Q_OBJECT;
   public:
      FormSubdialogSceneTimerAction(dovah::loaded_forms::Scene&, QWidget* parent = nullptr);

      struct {
         float duration = 0.0F;
         struct {
            std::string scriptname;
            std::string function;
         } fragment;
      } data;

      // After you write to `data`, call this to push state to the UI.
      virtual void refresh() override;
      
   protected:
      Ui::FormSubdialogSceneTimerAction ui;
      struct {
         DKPapyrusBoundScriptListPane* script_list_pane = nullptr; // HACK HACK HACK
      } _hidden_widgets;
};