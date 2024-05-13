#include "./DKCompactObjectReferencePickerDialog.h"
#include "dovah/forms/factories/hardcoded.h"
#include "editor/core.h"

DKCompactObjectReferencePickerDialog::DKCompactObjectReferencePickerDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);
   
   this->ui.refPicker->setAllowNone(true);

   QObject::connect(this->ui.buttonSelectPlayer, &QPushButton::clicked, this, [this]() {
      auto& editor = DovahKitCore::get();
      auto* form   = editor.get_form(dovah::hardcoded_form_ids::PlayerRef);
      this->ui.refPicker->setRef(form);
   });
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
}


dovah::form_stub* DKCompactObjectReferencePickerDialog::value() const {
   return this->ui.refPicker->ref();
}
void DKCompactObjectReferencePickerDialog::setValue(dovah::form_stub* ref) {
   this->ui.refPicker->setRef(ref);
}

dovah::form_type DKCompactObjectReferencePickerDialog::requiredFormType() const {
   return this->ui.refPicker->requiredFormType();
}
void DKCompactObjectReferencePickerDialog::setRequiredFormType(dovah::form_type ft) {
   this->ui.refPicker->setRequiredFormType(ft);
}

const std::string& DKCompactObjectReferencePickerDialog::requiredScriptname() const {
   return this->ui.refPicker->requiredScriptname();
}
void DKCompactObjectReferencePickerDialog::setRequiredScriptname(QString v) {
   this->ui.refPicker->setRequiredScriptname(v);
}
void DKCompactObjectReferencePickerDialog::setRequiredScriptname(std::string_view v) {
   this->ui.refPicker->setRequiredScriptname(v);
}