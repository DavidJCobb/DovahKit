#include "./FormSubdialogIdleNewActionRoot.h"
#include "./IdleNewActionRootPickerFilter.h"
#include "editor/form_stub_meta_type.h" // so QObject::connect doesn't choke on DKFormPicker

FormSubdialogIdleNewActionRoot::FormSubdialogIdleNewActionRoot(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   this->_action_filter = new IdleNewActionRootPickerFilter(this);
   this->ui.action->setAllowedFormType(dovah::form_type::action);
   this->ui.action->setCustomFilter(this->_action_filter);

   QObject::connect(this->ui.action,   &DKFormPicker::formChanged, this, &FormSubdialogIdleNewActionRoot::_update_ok_button_enable_state);
   QObject::connect(this->ui.editorID, &QLineEdit::textChanged,    this, &FormSubdialogIdleNewActionRoot::_update_ok_button_enable_state);
   this->_update_ok_button_enable_state();

   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void FormSubdialogIdleNewActionRoot::setExistingActionRoots(const std::vector<dovah::form_stub*>& actions) {
   this->_action_filter->setActions(actions);
}
void FormSubdialogIdleNewActionRoot::setExistingActionRoots(std::vector<dovah::form_stub*>&& actions) {
   this->_action_filter->setActions(std::move(actions));
}

dovah::form_stub* FormSubdialogIdleNewActionRoot::action() const {
   return this->ui.action->formStub();
}
QString FormSubdialogIdleNewActionRoot::idleEditorID() const {
   return this->ui.editorID->text();
}

void FormSubdialogIdleNewActionRoot::_update_ok_button_enable_state() {
   if (!this->ui.action->formStub()) {
      this->ui.buttonOK->setEnabled(false);
      return;
   }
   if (this->ui.editorID->text().isEmpty()) {
      this->ui.buttonOK->setEnabled(false);
      return;
   }
   this->ui.buttonOK->setEnabled(true);
}