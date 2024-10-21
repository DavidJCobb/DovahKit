#pragma once
#include <vector>
#include <QDialog>
#include "ui_FormSubdialogScenePhase.h" // generated
#include "ui/types/conditions/condition.h"

namespace dovah::loaded_forms {
   class Scene;
}

class FormSubdialogScenePhase : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type = dovah::loaded_forms::Scene;
      struct phase_fragment {
         QString scriptname;
         QString function;
      };

   public:
      FormSubdialogScenePhase(loaded_form_type&, QWidget* parent = nullptr);

      loaded_form_type* loaded_form = nullptr;
      struct {
         QString name;
         struct {
            std::vector<ui::types::conditions::condition> start;
            std::vector<ui::types::conditions::condition> completion;
         } conditions;
         struct {
            phase_fragment start;
            phase_fragment completion;
         } fragments;
      } data;

      // After you write to `data`, call this to push state to the UI.
      void refresh();
      
   protected:
      Ui::FormSubdialogScenePhase ui;
      struct {
         DKPapyrusBoundScriptListPane* script_list_pane = nullptr; // HACK HACK HACK
      } _hidden_widgets;
};