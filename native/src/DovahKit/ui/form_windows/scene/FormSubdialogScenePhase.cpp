#include "./FormSubdialogScenePhase.h"
#include <array>
#include "dovah/forms/Scene.h"
#include "widgets/DKPapyrusBoundScriptListPane.h"

FormSubdialogScenePhase::FormSubdialogScenePhase(loaded_form_type& scene, QWidget* parent) : QDialog(parent), loaded_form(&scene) {
   this->ui.setupUi(this);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   //
   // HACK for the Papyrus fragment pickers: give them a hidden script-list-pane they 
   // can read from.
   //
   this->_hidden_widgets.script_list_pane = new DKPapyrusBoundScriptListPane(this);
   this->_hidden_widgets.script_list_pane->setFormWorkingCopy(&scene);
   this->ui.startFragment->setSourceWidget(this->_hidden_widgets.script_list_pane);
   this->ui.completionFragment->setSourceWidget(this->_hidden_widgets.script_list_pane);

   QObject::connect(this->ui.flagEndOnConditions, &QRadioButton::toggled, this, [this](bool checked) {
      this->ui.completionConditions->setEnabled(checked);
   });

   this->ui.tabWidget->setCurrentIndex(0);

   QObject::connect(this, &QDialog::accepted, this, [this]() {
      this->data.name = this->ui.name->text();
      this->ui.startConditions->exportTo(*this->loaded_form, this->data.conditions.start);
      if (this->ui.flagEndOnConditions->isChecked()) {
         this->ui.completionConditions->exportTo(*this->loaded_form, this->data.conditions.completion);
      } else {
         this->data.conditions.completion.clear();
      }

      this->data.fragments.start = {
         .scriptname = this->ui.startFragment->currentScriptname(),
         .function   = this->ui.startFragment->currentFunction(),
      };
      this->data.fragments.completion = {
         .scriptname = this->ui.completionFragment->currentScriptname(),
         .function   = this->ui.completionFragment->currentFunction(),
      };
   });
}
void FormSubdialogScenePhase::refresh() {
   const auto blockers = std::array{
      QSignalBlocker(this->ui.name),
   };

   this->ui.name->setText(this->data.name);
   this->ui.startConditions->importFrom(*this->loaded_form, this->data.conditions.start);
   this->ui.completionConditions->importFrom(*this->loaded_form, this->data.conditions.completion);
   if (this->data.conditions.completion.empty()) {
      this->ui.flagEndOnConditions->setChecked(false);
   } else {
      this->ui.flagEndOnConditions->setChecked(true);
   }

   this->ui.startFragment->setCurrentScriptname(this->data.fragments.start.scriptname);
   this->ui.startFragment->setCurrentFunction(this->data.fragments.start.function);
   this->ui.completionFragment->setCurrentScriptname(this->data.fragments.completion.scriptname);
   this->ui.completionFragment->setCurrentFunction(this->data.fragments.completion.function);
}