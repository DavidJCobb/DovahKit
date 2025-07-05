#include "./FormSubdialogFactionVendorLocation.h"

FormSubdialogFactionVendorLocation::FormSubdialogFactionVendorLocation(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.nearLinkedRef, &QRadioButton::toggled, this, [this](bool checked) {
      this->ui.nearLinkedRefKeyword->setEnabled(checked);
   });
   QObject::connect(this->ui.nearCell, &QRadioButton::toggled, this, [this](bool checked) {
      this->ui.nearCellForm->setEnabled(checked);
   });
   QObject::connect(this->ui.nearRef, &QRadioButton::toggled, this, [this](bool checked) {
      this->ui.nearRefForm->setEnabled(checked);
   });

   this->ui.nearCellForm->setAllowedFormType(dovah::form_type::cell);
   this->ui.nearLinkedRefKeyword->setAllowedFormType(dovah::form_type::keyword);

   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   // force UI enable state updates:
   this->ui.nearLinkedRef->toggled(false);
   this->ui.nearCell->toggled(false);
   this->ui.nearRef->toggled(false);
   this->ui.nearEditorLoc->setChecked(true);
}

void FormSubdialogFactionVendorLocation::importFrom(const location_type& src) {
   switch (src.get_type()) {
      using enum dovah::packages::location_type;
      case reference:
         this->ui.nearRef->setChecked(true);
         this->ui.nearRefForm->setRef(src.as_type<reference>()->get_form_stub());
         break;
      case interior_cell:
         this->ui.nearCell->setChecked(true);
         this->ui.nearCellForm->setFormStub(src.as_type<interior_cell>()->get_form_stub());
         break;
      case near_package_start_location:
         this->ui.nearPackStart->setChecked(true);
         break;
      case near_editor_location:
         this->ui.nearEditorLoc->setChecked(true);
         break;
      case linked_ref:
         this->ui.nearLinkedRef->setChecked(true);
         this->ui.nearLinkedRefKeyword->setFormStub(src.as_type<linked_ref>()->get_form_stub());
         break;
      case self:
         this->ui.nearSelf->setChecked(true);
         break;

      // Unsupported types:
      case at_package_location: // disabled in CK
      case object:
      case object_type:
      case reference_alias: // disabled in CK
      case location_alias: // disabled in CK
      default:
         //
         // Fall back to "Near Editor Location."
         //
         this->ui.nearPackStart->setChecked(true);
         break;
   }
}
void FormSubdialogFactionVendorLocation::commitTo(loaded_form_type& dst_owner, location_type& dst) const {
   if (this->ui.nearRef->isChecked()) {
      dst.set_type(dst_owner, dovah::packages::location_type::reference);
      dst.as_type<dovah::packages::location_type::reference>()->set(dst_owner, this->ui.nearRefForm->ref());
   } else if (this->ui.nearCell->isChecked()) {
      dst.set_type(dst_owner, dovah::packages::location_type::interior_cell);
      dst.as_type<dovah::packages::location_type::interior_cell>()->set(dst_owner, this->ui.nearCellForm->formStub());
   } else if (this->ui.nearLinkedRef->isChecked()) {
      dst.set_type(dst_owner, dovah::packages::location_type::linked_ref);
      dst.as_type<dovah::packages::location_type::linked_ref>()->set(dst_owner, this->ui.nearLinkedRefKeyword->formStub());
   } else if (this->ui.nearPackStart->isChecked()) {
      dst.set_type(dst_owner, dovah::packages::location_type::near_package_start_location);
   } else if (this->ui.nearEditorLoc->isChecked()) {
      dst.set_type(dst_owner, dovah::packages::location_type::near_editor_location);
   } else if (this->ui.nearSelf->isChecked()) {
      dst.set_type(dst_owner, dovah::packages::location_type::self);
   } else {
      //
      // Fall back to "Near Editor Location."
      //
      #if _DEBUG
         __debugbreak(); // ...but debugbreak since the UI shouldn't be able to get into this state.
      #endif
      dst.set_type(dst_owner, dovah::packages::location_type::near_editor_location);
   }
}