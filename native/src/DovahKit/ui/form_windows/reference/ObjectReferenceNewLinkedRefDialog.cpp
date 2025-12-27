#include "./ObjectReferenceNewLinkedRefDialog.h"
#include "../shared/DKFormPickerExcludeListedFormsFilter.h"

ObjectReferenceNewLinkedRefDialog::ObjectReferenceNewLinkedRefDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   this->keyword_filter = new DKFormPickerExcludeListedFormsFilter(this);
   this->ui.keyword->setCustomFilter(this->keyword_filter);

   this->ui.ref->setValidationFunction([this](dovah::form_stub* ref) -> bool {
      return ref != this->disallowed_ref;
   });
}

dovah::form_stub* ObjectReferenceNewLinkedRefDialog::keyword() const {
   return this->ui.keyword->formStub();
}
dovah::form_stub* ObjectReferenceNewLinkedRefDialog::ref() const {
   return this->ui.ref->ref();
}

void ObjectReferenceNewLinkedRefDialog::setDisallowedKeywords(std::vector<dovah::form_stub*>&& keywords) {
   this->keyword_filter->set_exclusion(std::move(keywords));
}
void ObjectReferenceNewLinkedRefDialog::setDisallowedRef(dovah::form_stub& ref) {
   this->disallowed_ref = &ref;
   if (this->ui.ref->ref() == &ref) {
      this->ui.ref->setRef(nullptr);
   }
}