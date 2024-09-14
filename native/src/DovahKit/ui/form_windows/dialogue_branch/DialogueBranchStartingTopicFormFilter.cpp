#include "./DialogueBranchStartingTopicFormFilter.h"
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_branch.h"

/*virtual*/ bool DialogueBranchStartingTopicFormFilter::form_matches(const dovah::form_stub& stub) const noexcept /*override*/ {
   if (stub.form_type != dovah::form_type::topic)
      return false;
   if (dovah::form_stub_helpers::get_dialogue_topic_branch(&stub) != this->branch)
      return false;
   return true;
}

void DialogueBranchStartingTopicFormFilter::setDialogueBranch(const dovah::form_stub* stub) {
   if (stub == this->branch)
      return;
   if (stub && stub->form_type != dovah::form_type::dialogue_branch)
      return;
   this->branch = stub;
   this->_refilter_all_forms();
}