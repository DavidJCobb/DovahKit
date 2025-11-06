#include "./IdleNewActionRootPickerFilter.h"
#include "dovah/form_stub.h"

/*virtual*/ bool IdleNewActionRootPickerFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   auto it = std::find(this->actions_to_exclude.begin(), this->actions_to_exclude.end(), &stub);
   return (it == this->actions_to_exclude.end());
}