#include "./FormSubdialogClimateWeather.h"

FormSubdialogClimateWeather::FormSubdialogClimateWeather(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);
   this->ui.weather->setAllowedFormType(dovah::form_type::weather);
   this->ui.global->setAllowedFormType(dovah::form_type::global);

   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
}
void FormSubdialogClimateWeather::setValue(const value_type& v) {
   this->ui.weather->setFormStub(v.weather);
   this->ui.chance->setValue(v.chance);
   this->ui.global->setFormStub(v.global);
}
FormSubdialogClimateWeather::value_type FormSubdialogClimateWeather::value() const {
   return value_type{
      .weather = this->ui.weather->formStub(),
      .global  = this->ui.global->formStub(),
      .chance  = this->ui.chance->value(),
   };
}