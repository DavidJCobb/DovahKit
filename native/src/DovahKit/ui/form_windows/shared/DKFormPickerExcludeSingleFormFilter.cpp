#include "./DKFormPickerExcludeSingleFormFilter.h"

/*virtual*/ bool DKFormPickerExcludeSingleFormFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   return &stub != this->_exclude;
}

void DKFormPickerExcludeSingleFormFilter::set_exclusion(dovah::form_stub* exclude) {
   if (exclude == this->_exclude)
      return;
   auto* prior = this->_exclude;
   this->_exclude = exclude;
   if (prior)
      this->_refilter_form(*prior);
   if (exclude)
      this->_refilter_form(*exclude);
}