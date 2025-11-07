#include "./FormSubdialogIdleNewActionRoot.h"
#include "./IdleNewActionRootPickerFilter.h"

FormSubdialogIdleNewActionRoot::FormSubdialogIdleNewActionRoot(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   this->_action_filter = new IdleNewActionRootPickerFilter(this);
   this->ui.action->setAllowedFormType(dovah::form_type::action);
   this->ui.action->setCustomFilter(this->_action_filter);

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