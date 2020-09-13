#include "FormsOfTypeCombobox.h"
#include "../../dovah/form_stub.h"
#include "../../editor/core.h"

//
// Constructing these and populating them can incur a pretty darned heavy performance hit. 
// I wonder... What if we built a single persistent model that handled all forms, and then 
// gave every FormsOfTypeCombobox a proxy model which wrapped it and hid irrelevant forms? 
// Failing that, what if we just had a persistent model for every form type, such that two 
// FormsOfTypeComboboxes focusing on the same single form type could share models? (That 
// would be less useful for FormsOfTypeComboxes that have to support multiple form types 
// at a time.)
//

void FormsOfTypeCombobox::addFormType(dovah::form_type_t ft) {
   this->_formTypes.push_back(ft);
}
bool FormsOfTypeCombobox::allowsFormType(dovah::form_type_t ft) const noexcept {
   return this->_formTypes.indexOf(ft) >= 0;
}
dovah::bare_form_id_t FormsOfTypeCombobox::formID() const noexcept {
   int i = this->currentIndex();
   if (i < 0)
      return 0;
   return this->currentData().toUInt();
}
void FormsOfTypeCombobox::populate() {
   const auto blocker = QSignalBlocker(this);
   this->clear();
   //
   auto& editor = DovahKitCore::get();
   if (!editor.has_data())
      return;
   for (auto ft : this->_formTypes) {
      editor.for_each_form_of_type(ft, [this](dovah::form_stub* stub) {
         this->addItem(QString::fromStdString(stub->get_editor_id()), stub->formID);
         return false;
      });
   }
   emit populated();
}
void FormsOfTypeCombobox::setAllowNone(bool s) noexcept {
   if (this->_allowNone == s)
      return;
   this->_allowNone = s;
   if (s) {
      this->addItem(tr("NONE"), 0);
   } else {
      int i = this->findData(0);
      if (i >= 0)
         this->removeItem(i);
   }
}
void FormsOfTypeCombobox::setFormByID(dovah::bare_form_id_t formID) noexcept {
   int i = this->findData(formID);
   if (i >= 0)
      this->setCurrentIndex(i);
}

/*static*/ void FormsOfTypeCombobox::populate(dovah::form_type_t ft, QVector<FormsOfTypeCombobox*>& widgets) {
   for (auto*& widget : widgets) {
      widget->clear();
      widget->blockSignals(true);
      if (!widget->allowsFormType(ft)) {
         widget->blockSignals(false);
         widget = nullptr;
      }
   }
   auto& editor = DovahKitCore::get();
   if (!editor.has_data())
      return;
   editor.for_each_form_of_type(ft, [&widgets](dovah::form_stub* stub) {
      for (auto widget : widgets)
         widget->addItem(QString::fromStdString(stub->get_editor_id()), stub->formID);
      return false;
   });
   for (auto* widget : widgets)
      widget->blockSignals(false);
}