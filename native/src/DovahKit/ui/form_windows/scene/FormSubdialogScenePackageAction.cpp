#include "./FormSubdialogScenePackageAction.h"

FormSubdialogScenePackageAction::FormSubdialogScenePackageAction(QWidget* parent) : FormSubdialogSceneActionBase(parent) {
   this->ui.setupUi(this);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   {
      auto* widget = this->ui.packages;
      widget->setAllowedFormTypes({ dovah::form_type::package });
   }

   QObject::connect(this, &QDialog::accepted, this, [this]() {
      auto src = this->ui.packages->stubs();
      this->data.packages = { src.begin(), src.end() };
   });
}
void FormSubdialogScenePackageAction::refresh() {
   this->_refresh_base(
      this->ui.name,
      this->ui.actor,
      this->ui.phaseStart,
      this->ui.phaseEnd
   );
   {
      auto* widget = this->ui.packages;
      widget->reserve(this->data.packages.size());
      for (auto* item : this->data.packages)
         widget->addStub(item);
   }
}