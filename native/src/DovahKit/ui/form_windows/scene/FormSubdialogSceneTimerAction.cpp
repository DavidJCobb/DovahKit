#include "./FormSubdialogSceneTimerAction.h"
#include <limits>
#include "dovah/forms/Scene.h"
#include "widgets/DKPapyrusBoundScriptListPane.h"

FormSubdialogSceneTimerAction::FormSubdialogSceneTimerAction(dovah::loaded_forms::Scene& scene, QWidget* parent) : FormSubdialogSceneActionBase(parent) {
   this->ui.setupUi(this);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   this->ui.duration->setRange(0, std::numeric_limits<float>::max());
   //
   // HACK for the Papyrus fragment pickers: give them a hidden script-list-pane they 
   // can read from.
   //
   this->_hidden_widgets.script_list_pane = new DKPapyrusBoundScriptListPane(this);
   this->_hidden_widgets.script_list_pane->setFormWorkingCopy(&scene);
   this->ui.fragment->setSourceWidget(this->_hidden_widgets.script_list_pane);

   QObject::connect(this, &QDialog::accepted, this, [this]() {
      this->data.duration = this->ui.duration->value();
      this->data.fragment = {
         .scriptname = this->ui.fragment->currentScriptname().toStdString(),
         .function   = this->ui.fragment->currentFunction().toStdString(),
      };
   });
}
void FormSubdialogSceneTimerAction::refresh() {
   this->_refresh_base(
      this->ui.name,
      this->ui.actor,
      this->ui.phaseStart,
      this->ui.phaseEnd
   );
   this->ui.duration->setValue(this->data.duration);
   this->ui.fragment->setCurrentScriptname(this->data.fragment.scriptname);
   this->ui.fragment->setCurrentFunction(this->data.fragment.function);
}