#include "./FormPickerFromFormListPaneFilter.h"
#include "widgets/DKFormListPane.h"

/*virtual*/ bool FormPickerFromFormListPaneFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   if (!this->_pane) {
      return false;
   }
   return this->_pane->contains(&stub);
}

DKFormListPane* FormPickerFromFormListPaneFilter::pane() const {
   return this->_pane;
}
void FormPickerFromFormListPaneFilter::setPane(DKFormListPane* pane) {
   if (this->_pane == pane)
      return;
   this->_set_pane_impl(pane);
}

void FormPickerFromFormListPaneFilter::_set_pane_impl(DKFormListPane* pane) {
   if (DKFormListPane* prior = this->_pane) {
      QObject::disconnect(prior, nullptr, this, nullptr);
   }
   this->_pane = pane;
   if (pane) {
      QObject::connect(pane, &DKFormListPane::formsAdded,   this, &FormPickerFromFormListPaneFilter::_refilter_all_forms);
      QObject::connect(pane, &DKFormListPane::formsRemoved, this, &FormPickerFromFormListPaneFilter::_refilter_all_forms);
   }
   this->_refilter_all_forms();
}