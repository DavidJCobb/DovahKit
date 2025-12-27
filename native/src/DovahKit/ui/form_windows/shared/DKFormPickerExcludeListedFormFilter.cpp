#include "./DKFormPickerExcludeListedFormsFilter.h"

/*virtual*/ bool DKFormPickerExcludeListedFormsFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   for (auto* excluded : this->_exclude)
      if (excluded == &stub)
         return false;
   return true;
}

void DKFormPickerExcludeListedFormsFilter::add_exclusion(dovah::form_stub& exclude) {
   auto it = std::find(this->_exclude.begin(), this->_exclude.end(), &exclude);
   if (it != this->_exclude.end())
      return;
   this->_exclude.push_back(&exclude);
   this->_refilter_form(exclude);
}
void DKFormPickerExcludeListedFormsFilter::set_exclusion(std::vector<dovah::form_stub*>&& list) {
   this->_exclude = std::move(list);
   this->_refilter_all_forms();
}
void DKFormPickerExcludeListedFormsFilter::set_exclusion(const std::vector<dovah::form_stub*>& list) {
   this->_exclude = list;
   this->_refilter_all_forms();
}