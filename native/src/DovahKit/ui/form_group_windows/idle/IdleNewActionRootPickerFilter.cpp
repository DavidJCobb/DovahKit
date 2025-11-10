#include "./IdleNewActionRootPickerFilter.h"
#include "dovah/form_stub.h"

/*virtual*/ bool IdleNewActionRootPickerFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   auto it = std::find(this->actions_to_exclude.begin(), this->actions_to_exclude.end(), &stub);
   return (it == this->actions_to_exclude.end());
}

void IdleNewActionRootPickerFilter::setActions(std::vector<dovah::form_stub*>&& list) {
   this->actions_to_exclude = std::move(list);
   this->_refilter_all_forms();
}
void IdleNewActionRootPickerFilter::setActions(const std::vector<dovah::form_stub*>& list) {
   this->actions_to_exclude = list;
   this->_refilter_all_forms();
}